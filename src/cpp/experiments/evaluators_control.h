#ifndef evaluators_control_h
#define evaluators_control_h

#include <TPG.h>

#include "EvalData.h"

/******************************************************************************/
void MaybeStartAnimation(TPG &tpg) {
    (void)tpg;
#if !defined(CCANADA) && !defined(HPCC)
    if (tpg.GetParam<int>("animate")) {
        double _width = 1200;
        double _height = 1200;
        int argc = 1;
        char *argv[1] = {(char *)"null"};
        glutInit(&argc, argv);
        // glutInitDisplayMode(GLUT_SINGLE |GLUT_RGB);
        glutInitWindowSize(_width, _height);
        glutInitWindowPosition(100, 100);
        glutCreateWindow("classicRL");
        // glutHideWindow();
        glScalef(0.5, 0.5, 0.0);
    }
#endif
}

/******************************************************************************/
void MaybeAnimateStep(EvalData &eval) {
    (void)eval;
#if !defined(CCANADA)
    if (eval.animate) {
        eval.task->display_function(eval.episode, WrapDiscreteAction(eval),
                                    WrapContinuousAction(eval));
        char filename[80];
        sprintf(filename, "%s_%05d_%03d_%05d_%05d_%05d.tga", "replay/frames/gl",
                eval.save_frame++, eval.episode, eval.task->step, 0, 0);
        eval.task->saveScreenshotToFile(filename, 1200, 1200);
        // this_thread::sleep_for(std::chrono::milliseconds(10)); TODO(skelly): add
    }
#endif
}

/******************************************************************************/
void EvalControl(TPG &tpg, EvalData &eval) {
    eval.task->reset(tpg.rngs_[AUX_SEED]);
    eval.n_prediction = 0;
    state *obs = new state(tpg.n_input_[tpg.GetState("active_task")]);
    obs->Set(eval.task->GetObsVec(eval.partially_observable));
    while (!eval.task->terminal()) {
        eval.program_out = tpg.getAction(
            eval.tm, obs, true, eval.teams_visited, eval.instruction_count,
            eval.task->step, eval.team_path, tpg.rngs_[AUX_SEED], false);

        MaybeAnimateStep(eval);
        TaskEnv::Results r =
            eval.task->update(WrapDiscreteAction(eval),
                              WrapContinuousAction(eval), tpg.rngs_[AUX_SEED]);
        eval.stats_double[REWARD1_IDX] += r.r1;
        eval.AccumulateStepData();
        eval.n_prediction++;
        obs->Set(eval.task->GetObsVec(eval.partially_observable));
    }
    MaybeAnimateStep(eval);
    delete obs;
}

/******************************************************************************/
void EvalControlViz(TPG &tpg, EvalData &eval,
                    vector<map<long, double>> &teamUseMapPerTask,
                    set<team *, teamIdComp> &teams_visitedAllTasks,
                    int &steps) {
    eval.task->reset(tpg.rngs_[AUX_SEED]);
    state *obs = new state(tpg.n_input_[tpg.GetState("active_task")]);
    obs->Set(eval.task->GetObsVec(eval.partially_observable));
    while (!eval.task->terminal()) {
        eval.program_out = tpg.getAction(
            eval.tm, obs, true, eval.teams_visited, eval.instruction_count,
            eval.task->step, eval.team_path, tpg.rngs_[AUX_SEED], false);
        eval.n_prediction++;
        for (auto tm : eval.teams_visited) {
            if (teamUseMapPerTask[tpg.state_["active_task"]].find(tm->id_) ==
                teamUseMapPerTask[tpg.state_["active_task"]].end()) {
                teamUseMapPerTask[tpg.state_["active_task"]][tm->id_] = 1.0;
            } else {
                teamUseMapPerTask[tpg.state_["active_task"]][tm->id_] += 1.0;
            }
        }
        steps++;
        // teamUseMapPerTask[tpg.state_["active_task"]][eval.tm->id_] += 1.0;
        teams_visitedAllTasks.insert(eval.teams_visited.begin(),
                                     eval.teams_visited.end());
        MaybeAnimateStep(eval);
        TaskEnv::Results r =
            eval.task->update(WrapDiscreteAction(eval),
                              WrapContinuousAction(eval), tpg.rngs_[AUX_SEED]);
        eval.stats_double[REWARD1_IDX] += r.r1;
        eval.AccumulateStepData();
        obs->Set(eval.task->GetObsVec(eval.partially_observable));
    }
    for (auto p : teamUseMapPerTask[tpg.state_["active_task"]]) {
        p.second = p.second / eval.task->step;
    }
    MaybeAnimateStep(eval);
    delete obs;
}

#endif