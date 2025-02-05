#ifndef EvalData_h
#define EvalData_h

#define REWARD1_IDX 0
#define VISITED_TEAMS_IDX 1
#define INSTRUCTIONS_IDX 2
#define REWARD2_IDX 3

#include "TPG.h"

/*******************************************************************************
 EvalData encapsulates all data relating to evaluation results.
 This simplifies data passing between evaluator processes.
*/
struct EvalData {
    vector<team *> teams;       // All teams to evaluate
    string checkpointString;    // For passing tpg data
    bool partially_observable;  // whether or not the task is partially
                                // observable

    /* Per individual eval data ***********************************************/
    team *tm;  // Current team to evaluate
    TaskEnv *task;
    string eval_result;       // For passing eval results between mpi procs
    int episode;              // Episode number
    vector<int> fingerprint;  // Behaviour fingerprint (usually not used)

    // double Stats to track for each evaluation
    // Currently this vec will contain:
    // [fitness, mean teams per prediction, mean instructions per prediction]
    vector<double> stats_double;

    // int Stats to track for each eavluation
    // Currently this vec will contain:
    // task number, phase, environment seed, internal test node id]
    vector<int> stats_int;

    // Programs which have won an auction in this eval
    set<RegisterMachine *, RegisterMachineIdComp> active;

    /* Per episode data *******************************************************/
    int n_prediction;  // Number or predictions in eval (if known)
    // Prediction and target sequences (for recursive forecasting)
    vector<double> sequence_targ;
    vector<double> sequence_pred;

    /* Per step data **********************************************************/
    int sample;          // Sample or step number
    int save_frame = 0;  // Whether to save animation frames
    vector<team *>
        team_path;  // Execution path (overwritten for each prediction)
    // Number of instructions executed per prediction (overwritten for each
    // prediction)
    long instruction_count;
    set<team *, teamIdComp> teams_visited;  // Overwritten for each prediction
    // Program which defines the output in the current timestep
    RegisterMachine *program_out;
    bool animate;            // whether or not to draw animation
    state *obs;              // Observation input
    list<double> obs_list;   // Obs as a list (tmp use only)
    vector<double> obs_vec;  // Obs as a vec (tmp use only)

    EvalData(TPG &tpg) {
        stats_double.resize(tpg.GetParam<int>("n_point_aux_double"));
        stats_int.resize(tpg.GetParam<int>("n_point_aux_int"));
        animate = tpg.GetParam<int>("animate") == 1;
        partially_observable = tpg.GetParam<int>("partially_observable") == 1;
        n_prediction = 0;
    }
    ~EvalData() {}

    void AccumulateStepData() {
        if (n_prediction == 0) {
            fill(stats_double.begin(), stats_double.end(), 0);
            fingerprint.clear();
        }
        stats_double[VISITED_TEAMS_IDX] += teams_visited.size();
        stats_double[INSTRUCTIONS_IDX] += instruction_count;
    }
    void FinalizeStepData(TPG &tpg) {
        if (task->eval_type_ == "RecursiveForecast") {
            if (tpg.GetParam<string>("forecast_fitness") == "mse") {
                auto err = MeanSquaredError(sequence_targ, sequence_pred);
                stats_double[REWARD1_IDX] = -err;
            } else if (tpg.GetParam<string>("forecast_fitness") ==
                       "correlation") {
                auto corr = Correlation(sequence_targ, sequence_pred);
                stats_double[REWARD1_IDX] = corr;
            } else if (tpg.GetParam<string>("forecast_fitness") == "pearson") {
                auto corr = PearsonCorrelation(sequence_targ, sequence_pred);
                stats_double[REWARD1_IDX] = corr;
            } else if (tpg.GetParam<string>("forecast_fitness") == "theils") {
                auto err = TheilsStatistic(sequence_targ, sequence_pred);
                stats_double[REWARD1_IDX] = -err;
            } else if (tpg.GetParam<string>("forecast_fitness") ==
                       "mse_multivar") {
                auto err = calculateMSE_Multi(sequence_targ, sequence_pred);
                stats_double[REWARD1_IDX] = -err;
            } else if (tpg.GetParam<string>("forecast_fitness") ==
                       "theils_multivar") {
                auto err = calculateTheils_Multi(sequence_targ, sequence_pred);
                stats_double[REWARD1_IDX] = -err;
            } else if (tpg.GetParam<string>("forecast_fitness") ==
                       "pearson_multivar") {
                auto corr =
                    calculatePearson_Multi(sequence_targ, sequence_pred);
                stats_double[REWARD1_IDX] = corr;
                if (!isfinite(stats_double[REWARD1_IDX]))
                    stats_double[REWARD1_IDX] = 0;
            } else {
                die(__FILE__, __FUNCTION__, __LINE__,
                    "Unsupported forecast fitness function");
            }
        }
        stats_double[VISITED_TEAMS_IDX] /= n_prediction;
        stats_double[INSTRUCTIONS_IDX] /= n_prediction;
        stats_int[POINT_AUX_INT_TASK] = tpg.GetState("active_task");
        stats_int[POINT_AUX_INT_PHASE] = tpg.GetState("phase");
        stats_int[POINT_AUX_INT_ENVSEED] = episode;
        stats_int[POINT_AUX_INT_internalTestNodeId] =
            tpg.GetState("internal_test_node_id");
        EncodeEvalResultString(tpg);
    }
    // eval_string stores all evaluation results that must be passed from
    // evaluator mpi jobs back to the main TPG process.
    void EncodeEvalResultString(TPG &tpg) {
        // eval_result.erase(std::remove(eval_result.begin(), eval_result.end(), '\0'), eval_result.end());
        eval_result += to_string(static_cast<long>(tm->id_));
        eval_result += ":4";// + VectorToStringNoSpace(fingerprint);  // 4?
        eval_result += ":0";// + to_string(tpg.GetState("active_task"));
        for (size_t r = 0; r < stats_double.size(); r++)
            eval_result += ":" + to_string(stats_double[r]);
        for (size_t r = 0; r < stats_int.size(); r++)
            eval_result += ":" + to_string(stats_int[r]);
        eval_result += "\n";
        // eval_result.erase(std::remove(eval_result.begin(), eval_result.end(), '\0'), eval_result.end());
    }

    // Method used by main TPG process to decode and incorporate eval data.
    static void DecodeEvalResultString(TPG &tpg, istringstream &f,
                                       vector<TaskEnv *> &tasks,
                                       std::map<long, team *> root_teams_map) {
        string line;
        vector<string> split_str;
        while (getline(f, line)) {
            vector<long> active;
            vector<double> r_stats_double;
            vector<int> r_stats_int;
            SplitString(line, ':', split_str);
            size_t s = 0;
            long rslt_id = atol(split_str[s++].c_str());
            string fingerprint = split_str[s++].c_str();
            tpg.params_["active_task"] = atoi(split_str[s++].c_str());
            for (int i = 0; i < tpg.GetParam<int>("n_point_aux_double"); i++)
                r_stats_double.push_back(atof(split_str[s++].c_str()));
            for (int i = 0; i < tpg.GetParam<int>("n_point_aux_int"); i++)
                r_stats_int.push_back(atoi(split_str[s++].c_str()));
            tpg.setOutcome(root_teams_map[rslt_id], fingerprint, r_stats_double,
                           r_stats_int, tpg.GetState("t_current"));
            // For control tasks, re-use training results as validation.
            // TODO(skelly): fix
            if (r_stats_int[POINT_AUX_INT_PHASE] == _TRAIN_PHASE &&
                (tasks[tpg.GetParam<int>("active_task")]->eval_type_ ==
                     "Control" ||
                 tasks[tpg.GetParam<int>("active_task")]->eval_type_ ==
                     "Mujoco")) {
                r_stats_int[POINT_AUX_INT_PHASE] = _VALIDATION_PHASE;
                tpg.setOutcome(root_teams_map[rslt_id], fingerprint,
                               r_stats_double, r_stats_int,
                               tpg.GetState("t_current"));
            }
        }
    }
};

#endif