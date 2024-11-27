#include <Acrobot.h>
#include <CartCentering.h>
#include <CartPole.h>
#include <MountainCar.h>
#include <MountainCarContinuous.h>
#include <Mujoco_Ant_v4.h>
#include <Mujoco_Half_Cheetah_v4.h>
#include <Mujoco_Hopper_v4.h>
#include <Mujoco_Inverted_Pendulum_v4.h>
#include <Mujoco_Reacher_v4.h>
#include <Mujoco_Humanoid_Standup_v4.h>
#include <Pendulum.h>
#include <RecursiveForecast.h>
#include <TPG.h>
#include <api_client.h>
#include <misc.h>
#include <phylogenetic_fitness.h>
#include <tpg_eval_mpi.h>

#include <algorithm>
#include <any>
#include <boost/mpi.hpp>
#include <chrono>
#include <cstdlib>

#include "evaluators_mujoco.h"

#define CHECKPOINT_MOD 1000000
#define PRINT_MOD 1
// rawfitness,  mean visitedTeams, decisionInstructions
#define NUM_POINT_AUX_DOUBLE 3
// task, phase, environment seed, internal test node id
#define NUM_POINT_AUX_INT 4
#define MODES_T 1000000000

int main(int argc, char** argv) {
   mpi::environment env(argc, argv);
   mpi::communicator world;
   TPG tpg;
   tpg.params_["id"] = -1;  // remove later
   tpg.state_["world_rank"] = world.rank();
   tpg.SetParams(argc, argv);

   APIClient* apiClient = nullptr;

   if (tpg.GetParam<int>("track_experiments")) {
      // Only instantiate APIClient if trackExperiment is true
      apiClient = new APIClient(getenv("COMET_API_KEY"),
                                tpg.GetParam<std::string>("experiment_key"));

      // Track run parameters
      cout << "Tracking experiment parameters" << endl;
      for (auto& param : tpg.params_) {
         std::string value;

         if (param.second.type() == typeid(int)) {
            value = std::to_string(std::any_cast<int>(param.second));
         } else if (param.second.type() == typeid(double)) {
            value = std::to_string(std::any_cast<double>(param.second));
         } else {
            value = std::any_cast<std::string>(param.second);
         }

         apiClient->LogParameter(param.first, value);
      }

      // Add experiment tracking to TPG
      tpg.InitExperimentTracking(apiClient);
   }

   ostringstream os;  // logging

   /****************************************************************************/
   // Read task sets from parameters and create environments.
   vector<TaskEnv*> tasks;
   stringstream ss(tpg.GetParam<string>("active_tasks"));
   while (ss.good()) {
      string substr;
      getline(ss, substr, ',');
      if (substr == "Cartpole")
         tasks.push_back(new CartPole());
      else if (substr == "Acrobot")
         tasks.push_back(new Acrobot());
      else if (substr == "CartCentering")
         tasks.push_back(new CartCentering());
      else if (substr == "Pendulum")
         tasks.push_back(new Pendulum());
      else if (substr == "MountainCar")
         tasks.push_back(new MountainCar());
      else if (substr == "MountainCarContinuous")
         tasks.push_back(new MountainCarContinuous());
      else if (substr == "Sunspots")
         tasks.push_back(new RecursiveForecast("Sunspots"));
      else if (substr == "Mackey")
         tasks.push_back(new RecursiveForecast("Mackey"));
      else if (substr == "Laser")
         tasks.push_back(new RecursiveForecast("Laser"));
      else if (substr == "Offset")
         tasks.push_back(new RecursiveForecast("Offset"));
      else if (substr == "Duration")
         tasks.push_back(new RecursiveForecast("Duration"));
      else if (substr == "Pitch")
         tasks.push_back(new RecursiveForecast("Pitch"));
      else if (substr == "PitchBach")
         tasks.push_back(new RecursiveForecast("PitchBach"));
      else if (substr == "Bach")
         tasks.push_back(new RecursiveForecast("Bach"));
      else if (substr == "Mujoco_Ant_v4")
         tasks.push_back(new Mujoco_Ant_v4(tpg.params_));
      else if (substr == "Mujoco_Inverted_Pendulum_v4")
         tasks.push_back(new Mujoco_Inverted_Pendulum_v4(tpg.params_));
      else if (substr == "Mujoco_Half_Cheetah_v4")
         tasks.push_back(new Mujoco_Half_Cheetah_v4(tpg.params_));
      else if (substr == "Mujoco_Reacher_v4")
         tasks.push_back(new Mujoco_Reacher_v4(tpg.params_));
      else if (substr == "Mujoco_Hopper_v4")
         tasks.push_back(new Mujoco_Hopper_v4(tpg.params_));
      else if (substr == "Mujoco_Humanoid_Standup_v4")
         tasks.push_back(new Mujoco_Humanoid_Standup_v4(tpg.params_));
      else {
         cerr << "Unrecognised task:" << substr << endl;
         exit(1);
      }

      if (tasks[tasks.size() - 1]->eval_type_ == "RecursiveForecast") {
         RecursiveForecast* task =
             dynamic_cast<RecursiveForecast*>(tasks.back());
         task->n_prime_ = tpg.GetParam<int>("forecast_prime_steps");
         task->n_predict_[0] = tpg.GetParam<int>("forecast_horizon_train");
         task->n_predict_[1] = tpg.GetParam<int>("forecast_horizon_val");
         task->n_predict_[2] = tpg.GetParam<int>("forecast_horizon_test");
         task->n_eval_train_ = tpg.GetParam<int>("forecast_n_eval_train");
         task->n_eval_val_ = tpg.GetParam<int>("forecast_n_eval_val");
         task->PrepareData(tpg.rngs_[TPG_SEED]);
         if (tpg.GetParam<int>("forecast_normalize_data")) {
            task->Normalize();
         }
         if (tpg.GetParam<int>("forecast_discrete")) {
            tpg.params_["n_discrete_action"] =
                int(task->uniq_discrete_univars_.size());
         }
      }
   }

   // Create task indices vector
   vector<int> taskIndices;
   for (int i = 0; i < (int)tasks.size(); i++)
      taskIndices.push_back(i);

   // Read number of inpts per task from parameters
   ss.clear();
   ss.str(tpg.GetParam<string>("n_input"));
   while (ss.good()) {
      string substr;
      getline(ss, substr, ',');
      tpg.n_input_.push_back(std::stoi(substr));
   }

   string allTaskString = "";
   for (size_t i = 0; i < tasks.size(); i++)
      allTaskString += to_string(i);

   tpg.state_["n_task"] = (int)tasks.size();
   tpg.state_["active_task"] = 0;
   tpg.params_["n_point_aux_double"] = NUM_POINT_AUX_DOUBLE;
   tpg.params_["n_point_aux_int"] = NUM_POINT_AUX_INT;
   tpg.state_["phase"] = _TRAIN_PHASE;
   if (world.rank() == 0) {
      os << "world_size " << world.size() << endl;
      os << "n_task " << tpg.GetState("n_task") << endl;
   }

   // placeholders for logging stats only
   set<team*, teamIdComp> visitedTeamsAll;
   vector<set<team*, teamIdComp>> visitedTeamsAllPerTask;
   set<team*, teamIdComp> visitedTeamsAllTasks;
   map<long, double>
       teamUseMap;  // maps team to frequency of use for a particular task;
   vector<map<long, double>> teamUseMapPerTask;
   teamUseMapPerTask.reserve(tasks.size());
   teamUseMapPerTask.resize(tasks.size());

   if (world.rank() == 0) {  // Master Process
      string my_string = "MAIN";

      // time logging
      auto startGen = chrono::system_clock::now();
      chrono::duration<double> endGen = chrono::system_clock::now() - startGen;
      auto startGenTeams = chrono::system_clock::now();
      chrono::duration<double> endGenTeams =
          chrono::system_clock::now() - startGenTeams;
      auto startSetEliteTeams = chrono::system_clock::now();
      chrono::duration<double> endSetEliteTeams;
      auto startSelTeams = chrono::system_clock::now();
      chrono::duration<double> endSelTeams;
      auto startEval = chrono::system_clock::now();
      chrono::duration<double> endEval;
      auto startChkp = chrono::system_clock::now();
      chrono::duration<double> endChkp;
      auto startMODES = chrono::system_clock::now();
      chrono::duration<double> endMODES;
      auto startReport = chrono::system_clock::now();
      chrono::duration<double> endReport;

      // Initialization //////////////////////////////////////////////////////
      if (tpg.GetParam<int>("start_from_checkpoint")) {
         tpg.ReadCheckpoint(tpg.GetParam<int>("checkpoint_in_t"),
                            tpg.GetParam<int>("checkpoint_in_phase"), -1, false,
                            "");
      } else {
         tpg.InitTeams();
      }

      // Main training loop.
      tpg.state_["phase"] = _TRAIN_PHASE;
      if (tpg.GetParam<int>("replay")) {
         tpg.state_["phase"] = _TEST_PHASE;
         tpg.state_["active_task"] = tpg.state_["task_to_replay"];
         tpg.ProcessParams();
         replayer_viz(tpg, tasks);
      } else {
         while (tpg.GetState("t_current") <=
                tpg.GetParam<int>("n_generations")) {
            // Replacement /////////////////////////////////////////////////
            if (tpg.GetState("t_current") > tpg.GetState("t_start")) {
               startGenTeams = chrono::system_clock::now();
               tpg.GenerateNewTeams();
               endGenTeams = chrono::system_clock::now() - startGenTeams;
            }

            // Evaluation //////////////////////////////////////////////////
            startEval = chrono::system_clock::now();
            tpg.MarkEffectiveCode();
            if (tpg.GetState("t_current") > tpg.GetState("t_start") &&
                tpg.HaveParam("n_sampled_tasks_for_eval")) {
               // Split tasks into evaluated and estimated
               vector<int> evalTasks, estTasks;
               SplitSet(taskIndices, evalTasks, estTasks,
                        tpg.GetParam<int>("n_sampled_tasks_for_eval"),
                        tpg.rngs_[TPG_SEED]);

               // Evaluate tasks
               evaluate_main(tpg, world, tasks, evalTasks);

               // Estimate remaining tasks with phylogeny
               estimate_main(tpg, tasks, estTasks);
            } else {
               // If first generation, evaluate on all tasks
               evaluate_main(tpg, world, tasks, taskIndices);
            }

            endEval = chrono::system_clock::now() - startEval;

            // Selection ///////////////////////////////////////////////////
            startSetEliteTeams = chrono::system_clock::now();
            tpg.SetEliteTeams(tasks);
            endSetEliteTeams = chrono::system_clock::now() - startSetEliteTeams;
            startSelTeams = chrono::system_clock::now();
            tpg.SelectTeams();
            endSelTeams = chrono::system_clock::now() - startSelTeams;

            // Accounting and reporting ////////////////////////////////////
            startReport = chrono::system_clock::now();
            if (tpg.GetParam<int>("test_mod") != 0 &&
                tpg.GetState("t_current") % tpg.GetParam<int>("test_mod") ==
                    0) {
               // validation
               tpg.state_["phase"] = _VALIDATION_PHASE;
               evaluate_main(tpg, world, tasks, taskIndices);
               tpg.SetEliteTeams(tasks);

               // test
               tpg.state_["phase"] = _TEST_PHASE;
               evaluate_main(tpg, world, tasks, taskIndices);
               tpg.SetEliteTeams(tasks);

               tpg.state_["phase"] = _TRAIN_PHASE;
            }
            endReport = chrono::system_clock::now() - startReport;

            /* MODES
             * *************************************************************/
            startMODES = chrono::system_clock::now();
            if (tpg.GetState("t_current") == tpg.GetState("t_start") ||
                tpg.GetState("t_current") % MODES_T == 0)
               tpg.updateMODESFilters(true);
            endMODES = chrono::system_clock::now() - startMODES;

            /* checkpoint
             * ********************************************************/
            startChkp = chrono::system_clock::now();
            if (tpg.GetParam<int>("write_train_checkpoints") &&
                tpg.GetState("t_current") % CHECKPOINT_MOD == 0) {
               // Checkpoint the entire population.
               tpg.WriteCheckpoint(tpg.GetState("t_current"), false);
            }
            if (tpg.GetParam<int>("write_phylogeny")) {
               tpg.printPhyloGraphDot(tpg.getBestTeam());
            }
            endChkp = chrono::system_clock::now() - startChkp;
            endGen = chrono::system_clock::now() - startGen;

            /* print generation timing
             * *******************************************/
            double lost = endGen.count() -
                          (endEval.count() + endGenTeams.count() +
                           endSetEliteTeams.count() + endSelTeams.count() +
                           endChkp.count() + endReport.count());

            if (tpg.GetParam<int>("track_experiments") &&
                tpg.GetState("t_current") % tpg.GetParam<int>("track_mod") ==
                    0) {
               std::string gen = to_string(tpg.GetState("t_current"));
               apiClient->LogMetric("sec", std::to_string(endGen.count()), "",
                                    gen);
               apiClient->LogMetric("evl", std::to_string(endEval.count()), "",
                                    gen);
               apiClient->LogMetric("gTms", std::to_string(endGenTeams.count()),
                                    "", gen);
               apiClient->LogMetric(
                   "elTms", std::to_string(endSetEliteTeams.count()), "", gen);
               apiClient->LogMetric("sTms", std::to_string(endSelTeams.count()),
                                    "", gen);
               apiClient->LogMetric("chkp", std::to_string(endChkp.count()), "",
                                    gen);
               apiClient->LogMetric("rprt", std::to_string(endReport.count()),
                                    "", gen);
               apiClient->LogMetric("MDS", std::to_string(endMODES.count()), "",
                                    gen);
               apiClient->LogMetric("lost", std::to_string(lost), "", gen);
            }
            os << setprecision(5) << fixed;
            os << "gTime t " << tpg.GetState("t_current");
            os << " sec " << endGen.count();
            os << " evl " << endEval.count();
            os << " gTms " << endGenTeams.count();
            os << " elTms " << endSetEliteTeams.count();
            os << " sTms " << endSelTeams.count();
            os << " chkp " << endChkp.count();
            os << " rprt " << endReport.count();
            os << " MDS " << endMODES.count();
            os << " lost " << lost;
            os << endl;
            tpg.printOss(os);

            startGen = chrono::system_clock::now();
            if (tpg.GetState("t_current") % PRINT_MOD == 0)
               tpg.printOss();
            tpg.SanityCheck();
            tpg.state_["t_current"]++;
         }
      }
      for (int ev = 1; ev <= world.size() - 1; ev++) {
         string d = "done";
         world.send(ev, 0, d);
      }
      tpg.printOss();
      cout << "Goodbye cruel world:" << world.rank() << endl;
   } else {  // Evaluator Process
      evaluator(tpg, world, tasks);
   }
   tpg.finalize();
   for (size_t tsk = 0; tsk < tasks.size(); tsk++)
      delete tasks[tsk];
   tasks.clear();
   return 0;
}
