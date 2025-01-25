#ifndef evaluators_forecast_h
#define evaluators_forecast_h

#include <TPG.h>

#include "EvalData.h"

void SaveRecursiveForecast(TPG &tpg, EvalData &eval) {
    RecursiveForecast *task = dynamic_cast<RecursiveForecast *>(eval.task);  
            auto targ = task->data[eval.sample + 1];
            auto pred = WrapVectorActionSigmoid(eval);
            for (size_t var = 0; var < targ.size(); var++) {
                eval.sequence_targ.push_back(targ[var]);    
                eval.sequence_pred.push_back(pred[var]);    
            }
            // // TODO(skelly):remove debugging output
            // cerr << "targ " << vecToStr(targ) << endl;
            // cerr << "pred " << vecToStr(pred) << endl;
}

void InitRecusiveForecastObs(TPG &tpg, EvalData &eval) {
    eval.sequence_targ.clear();
    eval.sequence_pred.clear();
    eval.obs = new state(tpg.n_input_[tpg.GetState("active_task")]);
    eval.obs_list.assign(tpg.n_input_[tpg.GetState("active_task")], 1.0);
    eval.obs_vec.assign(tpg.n_input_[tpg.GetState("active_task")], 1.0);
}

void PrepareRecursiveForecastObs(TPG &tpg, EvalData &eval, bool prime) {
    RecursiveForecast *task = dynamic_cast<RecursiveForecast *>(eval.task);
    if (prime) {  // prime
            eval.obs->Set(task->data[eval.sample]);
    } else {  // predict
            std::vector<double> v;
            v = WrapVectorActionSigmoid(eval);  // Previous action
            eval.obs->Set(v);
    }
}

/******************************************************************************/
void EvalRecursiveForecast(TPG &tpg, EvalData &eval) {
    RecursiveForecast *task = dynamic_cast<RecursiveForecast *>(eval.task);
    InitRecusiveForecastObs(tpg, eval);

    // Prime
    eval.sample = task->t_start[tpg.GetState("phase")][eval.episode];
    for (int i = 0; i < task->n_prime_ - 1; i++) {
        PrepareRecursiveForecastObs(tpg, eval, true);
        // Execute graph
        eval.program_out = tpg.getAction(
            eval.tm, eval.obs, true, eval.teams_visited, eval.instruction_count,
            eval.task->step_, eval.team_path, tpg.rngs_[AUX_SEED], false);
        eval.sample++;
    }
    // Predict
    for (eval.n_prediction = 0;
         eval.n_prediction < task->n_predict_[tpg.GetState("phase")];
         eval.n_prediction++) {
        PrepareRecursiveForecastObs(tpg, eval, false);
        // Execute graph
        eval.program_out = tpg.getAction(
            eval.tm, eval.obs, true, eval.teams_visited, eval.instruction_count,
            task->step_, eval.team_path, tpg.rngs_[AUX_SEED], false);
        SaveRecursiveForecast(tpg, eval);
        eval.sample++;
        eval.AccumulateStepData();
    }
    delete eval.obs;
}

void PrintRecursizeForecast(string filename, int t_start, int n_var,
                            vector<double> prime_samples,
                            vector<double> targets,
                            vector<double> predictions) {
    // Print csv format for quick plotting
    ofstream test_file;
    test_file.open(filename);
    // Print header
    test_file << "Time";
    for (int i = 0; i < n_var; i++) test_file << ",x" << to_string(i);
    for (int i = 0; i < n_var; i++) test_file << ",y" << to_string(i);
    test_file << endl;

    for (size_t i = 0; i < prime_samples.size() / n_var; i++) {
        test_file << t_start++;
        for (int var = 0; var < n_var; var++) {
            test_file << "," << prime_samples[i * n_var + var];
        }
        test_file << endl;
    }
    for (size_t i = 0; i < targets.size() / n_var; i++) {
        test_file << t_start++;
        for (int var = 0; var < n_var; var++) {
            test_file << std::fixed << "," << targets[i * n_var + var];
        }
        for (int var = 0; var < n_var; var++) {
            test_file << std::fixed << "," << predictions[i * n_var + var];
        }
        test_file << endl;
    }
    test_file.close();
}

/******************************************************************************/
void EvalRecursiveForecastViz(TPG &tpg, EvalData &eval,
                              vector<map<long, double>> &teamUseMapPerTask,
                              set<team *, teamIdComp> &teams_visitedAllTasks,
                              int &steps) {
    RecursiveForecast *task = dynamic_cast<RecursiveForecast *>(eval.task);
    eval.n_prediction = 0;
    
    InitRecusiveForecastObs(tpg, eval);

    // Prime
    vector<double> prime_samples_plot;
    eval.sample =
        task->t_start[tpg.GetParam<int>("checkpoint_in_phase")][eval.episode];
    for (int i = 0; i < task->n_prime_ - 1; i++) {
        PrepareRecursiveForecastObs(tpg, eval, true);
            prime_samples_plot.insert(prime_samples_plot.end(),
                                      task->data[eval.sample].begin(),
                                      task->data[eval.sample].end());
        // Execute graph
        eval.program_out = tpg.getAction(
            eval.tm, eval.obs, true, eval.teams_visited, eval.instruction_count,
            eval.task->step_, eval.team_path, tpg.rngs_[AUX_SEED], false);

        // Team user per task stats TODO(skelly): move to accumulator?
        for (auto tm : eval.teams_visited) {
            if (teamUseMapPerTask[tpg.state_["active_task"]].find(tm->id_) ==
                teamUseMapPerTask[tpg.state_["active_task"]].end()) {
                teamUseMapPerTask[tpg.state_["active_task"]][tm->id_] = 1.0;
            } else {
                teamUseMapPerTask[tpg.state_["active_task"]][tm->id_] += 1.0;
            }
        }
        // teamUseMapPerTask[tpg.state_["active_task"]][eval.tm->id_] += 1.0;
        teams_visitedAllTasks.insert(eval.teams_visited.begin(),
                                     eval.teams_visited.end());
        eval.sample++;
        steps++;
    }
    // Predict
    for (eval.n_prediction = 0;
         eval.n_prediction <
         task->n_predict_[tpg.GetParam<int>("checkpoint_in_phase")];
         eval.n_prediction++) {
        PrepareRecursiveForecastObs(tpg, eval, false);

        // Execute graph
        eval.program_out = tpg.getAction(
            eval.tm, eval.obs, true, eval.teams_visited, eval.instruction_count,
            task->step_, eval.team_path, tpg.rngs_[AUX_SEED], false);
        // Team user per task stats TODO(skelly): move to accumulator?
        for (auto tm : eval.teams_visited) {
            if (teamUseMapPerTask[tpg.state_["active_task"]].find(tm->id_) ==
                teamUseMapPerTask[tpg.state_["active_task"]].end()) {
                teamUseMapPerTask[tpg.state_["active_task"]][tm->id_] = 1.0;
            } else {
                teamUseMapPerTask[tpg.state_["active_task"]][tm->id_] += 1.0;
            }
        }
        // teamUseMapPerTask[tpg.state_["active_task"]][eval.tm->id_] += 1.0;
        teams_visitedAllTasks.insert(eval.teams_visited.begin(),
                                     eval.teams_visited.end());

        SaveRecursiveForecast(tpg, eval);
        eval.sample++;
        steps++;
        eval.AccumulateStepData();
    }
    delete eval.obs;
}

#endif