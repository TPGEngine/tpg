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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace mpi = boost::mpi;

typedef void (*EvaluatorFunction)(TPG &, EvalData &);

/*******************************************************************************
  Return a vector of the teams that need to be evaluated, depending on the
  current phase (train, test, validate) and current task
*/
inline vector<team *> GetTeamsToEval(TPG &tpg, TaskEnv *task) {
  auto root_teams = tpg.GetRootTeamsInVec();
  vector<team *> teams_to_eval;
  // Train and validate all teams.
  if (tpg.GetState("phase") != _TEST_PHASE) {
    for (auto tm : root_teams) {
      if (!tpg.GetParam<int>("keep_old_outcomes")) {
        tm->resetOutcomes(tpg.GetState("phase"));
      }
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
          tpg._eliteTeamPS[VectorToStringNoSpace(set)][tpg.GetParam<int>("fit_mode")]
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
inline void AssignTeamsToEvaluators(TPG &tpg, mpi::communicator &world,
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

      world.send(mpi_job, 0, s);
      mpi_job++;
      teams.clear();
      if (remainder > 0) remainder--;
    }
  }
}

/******************************************************************************/
inline bool NotDoneAndActive(EvalData &eval_data) {
  return eval_data.checkpointString.compare("x") != 0 &&
         eval_data.checkpointString.compare("done") != 0;
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
inline void evaluate_main(TPG &tpg, mpi::communicator &world, vector<TaskEnv *> &tasks,
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
  auto root_teams_map = tpg.GetRootTeamsInMap();
  all_strings.clear();
  gather(world, my_string, all_strings, 0);

  for (int proc = 1; proc < world.size(); proc++) {
    if (!all_strings[proc].empty()) {
      istringstream f(all_strings[proc]);
      tpg.DecodeEvalResultString(f, tasks, root_teams_map);
    }
  }
}

/*******************************************************************************
  This is the evaluator mpi job. It receive agents from main MPI job, evaluates
  each agent in the environment, and returns results to the main job.
 */
inline void evaluator(TPG &tpg, mpi::communicator &world, vector<TaskEnv *> &tasks) {
  unordered_map<string, EvaluatorFunction> evaluator_map;
  evaluator_map["Control"] = &EvalControl;
  evaluator_map["RecursiveForecast"] = &EvalRecursiveForecast;
  evaluator_map["Mujoco"] = &EvalMujoco;
  auto eval_data = tpg.InitEvalData();
  eval_data.world_rank = world.rank();
  eval_data.world_size = world.size();
  while (NotDoneAndActive(eval_data)) {
    world.recv(0, 0, eval_data.checkpointString);
    if (NotDoneAndActive(eval_data)) {
      tpg.ReadCheckpoint(-1, _TRAIN_PHASE, true, eval_data.checkpointString);
      eval_data.teams = tpg.GetRootTeamsInVec();
      eval_data.task = tasks[tpg.GetState("active_task")];
      eval_data.eval_result = "";
      for (auto tm : eval_data.teams) {
        eval_data.tm = tm;
        for (eval_data.episode = 0; eval_data.episode < eval_data.tm->_n_eval;
             eval_data.episode++) {   
           if (tpg.GetParam<int>("seed_with_episode_number")) {
              tpg.rngs_[AUX_SEED].seed(eval_data.episode * 42);
           }
          eval_data.tm->InitMemory(tpg.team_map_, tpg.params_);
          evaluator_map[eval_data.task->eval_type_](tpg, eval_data);
          // eval_data.FinalizeStepData(tpg);
          tpg.FinalizeStepData(eval_data);
        }
      }
      gather(world, eval_data.eval_result, 0);
    }
  }
}

/******************************************************************************/
inline void replayer(TPG &tpg, vector<TaskEnv *> &tasks) {
  // MaybeStartAnimation(tpg);
  auto eval_data = tpg.InitEvalData();

  vector<map<long, double>> teamUseMapPerTask;
  teamUseMapPerTask.resize(tpg.GetState("n_task"));
  std::set<team *, teamIdComp> teams_visitedAllTasks;

  eval_data.teams = tpg.GetRootTeamsInVec();
  eval_data.eval_result = "";
  for (auto tm : eval_data.teams) {
      if (tm->id_ != tpg.GetParam<int>("id_to_replay")) continue;
      eval_data.tm = tm;
      
      vector<int> steps_per_task(tpg.GetState("n_task"), 0);
      // TODO(skelly): clean up
      // tpg.rngs_[AUX_SEED].seed(tpg.GetParam<int>("seed_aux"));
      std::vector<double> outcomes;
      for (int task = 0; task < tpg.GetState("n_task"); task++) {
          tpg.state_["active_task"] = task;
          eval_data.task = tasks[tpg.GetState("active_task")];
          tm->_n_eval =
            eval_data.task->GetNumEval(_TEST_PHASE);
          for (eval_data.episode = 0; eval_data.episode < eval_data.tm->_n_eval;
              eval_data.episode++) {
                if (!eval_data.animate) {
                  tpg.rngs_[AUX_SEED].seed(eval_data.episode * 42);
                }
              eval_data.tm->InitMemory(tpg.team_map_, tpg.params_);

              if (eval_data.task->eval_type_ == "RecursiveForecast") {
                  EvalRecursiveForecastViz(
                      tpg, eval_data, teamUseMapPerTask, teams_visitedAllTasks,
                      steps_per_task[tpg.GetState("active_task")]);
              } else if (eval_data.task->eval_type_ == "Control") {
                  EvalControlViz(tpg, eval_data, teamUseMapPerTask,
                                teams_visitedAllTasks,
                                steps_per_task[tpg.GetState("active_task")]);
              } else {
                  EvalMujoco(tpg, eval_data);
              }
              // eval_data.FinalizeStepData(tpg);
              tpg.FinalizeStepData(eval_data);
              outcomes.push_back(eval_data.stats_double[REWARD1_IDX]);
          }
      }
      tpg.printGraphDotGPTPXXI(eval_data.tm->id_, teams_visitedAllTasks,
                              teamUseMapPerTask, steps_per_task);

      cout << " Evaluation result team:" << eval_data.tm->id_ << " n_outcomes "
          << outcomes.size() << " mean " << VectorMean(outcomes) << " median "
          << VectorMedian(outcomes) << endl;
      cout << VectorToString(outcomes) << endl;

    // Add video creation for headless mode
    if (headless && frame_idx > 0) {
      // Create videos directory if it doesn't exist
      struct stat st{};
      if (stat("videos", &st) == -1) {
          if (mkdir("videos", 0700) != 0) {
              cerr << "Failed to create videos directory" << endl;
              return;
          }
      }
      
      // using the tpg seed as the unique identifier
      string video_filename = "videos/video_" + to_string(eval_data.tpg_seed) + ".mp4";
      
      // Create video from frames using ffmpeg
      string ffmpeg_cmd = "ffmpeg -y -framerate 30 -i frames/frame_%05d.ppm -c:v libx264 -pix_fmt yuv420p " + video_filename;
      int ret = system(ffmpeg_cmd.c_str());
      if (ret != 0) {
          cerr << "Error creating video file" << endl;
      } else {
          cout << "Video saved to " << video_filename << endl;
      }
      
      // Clean up frame files
      ret = system("rm frames/frame_*.ppm");
      if (ret != 0) {
          cerr << "Error removing frame files" << endl;
      }
    }
  }
}

#endif