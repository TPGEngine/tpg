#ifndef tpg_eval_mpi_h
#define tpg_eval_mpi_h

#include <Acrobot.h>
#include <CartCentering.h>
#include <CartPole.h>
#include <MountainCar.h>
#include <MountainCarContinuous.h>
#include <Pendulum.h>
#include <TPG.h>
#include <TaskEnv.h>
#include <sequence_comparisons.h>
#include <ActionWrappers.h>
#include "evaluators_control.h"
#include "evaluators_forecast.h"
#include "evaluators_mujoco.h"

#include <boost/mpi.hpp>
#include <chrono>
#include <thread>
#include <GL/gl.h>
#include <GL/glut.h>

namespace mpi = boost::mpi;

typedef void (*EvaluatorFunction)(TPG &, EvalData &);

/*******************************************************************************
  Return a vector of the teams that need to be evaluated, depending on the
  current phase (train, test, validate) and current task
*/
vector<team *> GetTeamsToEval(TPG &tpg, TaskEnv *task) {
  auto root_teams = tpg.GetTeamsInVec(true);
  vector<team *> teams_to_eval;
  // Train and validate all teams.
  if (tpg.GetState("phase") != _TEST_PHASE) {
    for (auto tm : root_teams) {
      tm->_n_eval =
          task->GetNumEval(tpg.GetState("phase")) -
          tm->numOutcomes(tpg.GetState("phase"), tpg.GetState("active_task"));
      if (tm->_n_eval > 0) {
        teams_to_eval.push_back(tm);
      }
    }
  } else {
    // Test the single validation champion for each subset.
    auto PS = PowerSet(tpg.GetState("n_task"));
    for (auto &set : PS) {
      auto tm =
          tpg._eliteTeamPS[vecToStrNoSpace(set)][tpg.GetParam<int>("fit_mode")]
                          [_VALIDATION_PHASE];
      tm->_n_eval =
          task->GetNumEval(tpg.GetState("phase")) -
          tm->numOutcomes(tpg.GetState("phase"), tpg.GetState("active_task"));
      if (tm->_n_eval > 0) teams_to_eval.push_back(tm);
    }
  }
  return teams_to_eval;
}

/*******************************************************************************
 Given a vector of teams to evaluate, assign each team to a specific mpi job.
 Parameters:
  - tpg: TPG instance
  - world: MPI communicator object, which represents a group of processes that
    can communicate with each other
  - teams_to_eval: teams to evaluate
  - world_size_per_task: number of processors available to evaluate on this
    task
  - mpi_job: keeps track of current mpi job
 */
void AssignTeamsToEvaluators(TPG &tpg, mpi::communicator &world,
                             vector<team *> &teams_to_eval,
                             int world_size_per_task, int &mpi_job) {
  auto teams_per_evaluator = teams_to_eval.size() / world_size_per_task;
  auto remainder = teams_to_eval.size() % world_size_per_task;
  vector<team *> teams;
  // Assign teams_per_evaluator teams to each of world_size_per_task mpi jobs
  for (auto it = teams_to_eval.begin(); it != teams_to_eval.end(); it++) {
    teams.push_back(*it);
    if ((remainder > 0 && teams.size() == teams_per_evaluator + 1) ||
        (remainder == 0 && teams.size() == teams_per_evaluator) ||
        next(it) == teams_to_eval.end()) {
      string s = "";
      tpg.WriteMPICheckpoint(s, teams);

      // ofstream ofs;
      // char filename[80];
      // sprintf(filename, "eval_main.%d.rslt", tpg.GetState("t_current"));
      // ofs.open(filename, ios::out);
      // ofs << s;
      // ofs.close();

      world.send(mpi_job, 0, s);
      mpi_job++;
      teams.clear();
      if (remainder > 0) remainder--;
    }
  }
}

/******************************************************************************/
bool NotDoneAndActive(EvalData &eval) {
  return eval.checkpointString.compare("x") != 0 &&
         eval.checkpointString.compare("done") != 0;
}

/*******************************************************************************
 This is the man TPG process. What it does is:
 1. Assign agents to evaluator mpi jobs
   a. Partition available mpi jobs into groups for each task
   b. Each job in a group evaluates a subset of agents on the task
 2. Wait for evals to finish
 3. Collect results
 
 Parameters:
 - tpg The TPG instance with all the teams
 - world The MPI communicator object
 - tasks The set of all tasks in the TPG
 - eval_tasks The indices of the tasks to evaluate
*/
void evaluate_main(TPG &tpg, mpi::communicator &world, vector<TaskEnv *> &tasks,
                   vector<int> eval_tasks) {
  string my_string = "MAIN";
  vector<team *> teams_this_eval;
  vector<string> all_strings;
  string resultLine;

  int world_size_per_task = (world.size() - 1) / tasks.size();
  // Assign agents to evaluators
  int evaluator = 1;
  for (int task : eval_tasks) {
    tpg.state_["active_task"] = task;
    auto teams_to_eval = GetTeamsToEval(tpg, tasks[task]);
    AssignTeamsToEvaluators(tpg, world, teams_to_eval, world_size_per_task,
                            evaluator);
  }

  // Let the rest of the procs know they are not needed this round
  while (evaluator <= (world.size() - 1)) {
    world.send(evaluator++, 0, "x");
  }

  // Collect evaluation result from each evaluator
  auto root_teams_map = tpg.GetTeamsInMap(true);
  all_strings.clear();
  gather(world, my_string, all_strings, 0);

  for (int proc = 1; proc < world.size(); proc++) {
    if (!all_strings[proc].empty()) {
      istringstream f(all_strings[proc]);
      EvalData::DecodeEvalResultString(tpg, f, tasks, root_teams_map);
    }
  }
}

/*******************************************************************************
  This is the evaluator mpi job. It receive agents from main MPI job, evaluates
  each agent in the environment, and returns results to the main job.
 */
void evaluator(TPG &tpg, mpi::communicator &world, vector<TaskEnv *> &tasks) {
  unordered_map<string, EvaluatorFunction> evaluator_map;
  evaluator_map["Control"] = &EvalControl;
  evaluator_map["RecursiveForecast"] = &EvalRecursiveForecast;
  evaluator_map["Mujoco"] = &EvalMujoco;
  // MaybeStartAnimation(tpg);
  EvalData eval(tpg);
  while (NotDoneAndActive(eval)) {
    world.recv(0, 0, eval.checkpointString);
    if (NotDoneAndActive(eval)) {
      tpg.ReadCheckpoint(-1, _TRAIN_PHASE, -1, true, eval.checkpointString);
      tpg.getTeams(eval.teams, true);

      eval.task = tasks[tpg.GetState("active_task")];
      eval.eval_result = "";
      for (auto tm : eval.teams) {
        eval.tm = tm;
        for (eval.episode = 0; eval.episode < eval.tm->_n_eval;
             eval.episode++) {
          tpg.rngs_[AUX_SEED].seed(eval.episode);
          eval.tm->InitMemory(tpg._teamMap, tpg.params_);
          evaluator_map[eval.task->eval_type_](tpg, eval);
          eval.FinalizeStepData(tpg);
        }
      }
      gather(world, eval.eval_result, 0);
    }
  }
  // tpg.finalize();
}

/******************************************************************************/
void replayer_viz(TPG &tpg, vector<TaskEnv *> &tasks) {
  // MaybeStartAnimation(tpg);
  EvalData eval(tpg);

  vector<map<long, double>> teamUseMapPerTask;
  teamUseMapPerTask.resize(tpg.GetState("n_task"));
  std::set<team *, teamIdComp> teams_visitedAllTasks;

  tpg.getTeams(eval.teams, true);
  eval.eval_result = "";
  for (auto tm : eval.teams) {
    if (tm->id_ != tpg.GetParam<int>("id_to_replay")) continue;
    eval.tm = tm;

    vector<int> steps_per_task(tpg.GetState("n_task"), 0);
    // TODO(skelly): clean up
    for (int task = 0; task < tpg.GetState("n_task"); task++) {
        tpg.state_["active_task"] = task;
        eval.task = tasks[tpg.GetState("active_task")];
        if (eval.animate) {
            eval.tm->_n_eval = 1;
        } else {
            eval.tm->_n_eval =
                eval.task->GetNumEval(tpg.GetParam<int>("checkpoint_in_phase"));
        }
        for (eval.episode = 0; eval.episode < eval.tm->_n_eval;
             eval.episode++) {
            tpg.rngs_[AUX_SEED].seed(eval.episode);
            eval.tm->InitMemory(tpg._teamMap, tpg.params_);

            if (eval.task->eval_type_ == "RecursiveForecast") {
                EvalRecursiveForecastViz(
                    tpg, eval, teamUseMapPerTask, teams_visitedAllTasks,
                    steps_per_task[tpg.GetState("active_task")]);
            } else if (eval.task->eval_type_ == "Control") {
                EvalControlViz(tpg, eval, teamUseMapPerTask,
                               teams_visitedAllTasks,
                               steps_per_task[tpg.GetState("active_task")]);
            } else {
                EvalMujoco(tpg, eval);
            }
            eval.FinalizeStepData(tpg);
        }
    }
    tpg.printGraphDotGPTPXXI(eval.tm->id_, teams_visitedAllTasks,
                             teamUseMapPerTask, steps_per_task);
    cout << " Evaluation result team:" << eval.tm->id_ << 
      " score:" << eval.stats_double[REWARD1_IDX] << endl;

  }
}

#endif