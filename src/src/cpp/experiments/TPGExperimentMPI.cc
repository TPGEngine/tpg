#include <RecursiveForecast.h>
#include <TPG.h>
#include <TaskEnvFactory.h>
#include "experiment_runner.h"
#include "replay_runner.h"
#include "training_runner.h"

#include <algorithm>
#include <any>
#include <boost/mpi.hpp>
#include <chrono>
#include <cstdlib>

#include "storage/selection/selection_storage.h"
#include "loggers/selection/selection_logger.h"
#include "storage/timing/timing_storage.h"
#include "loggers/timing/timing_logger.h"
#include "core/event_dispatcher.h"
#include "metrics/timing/timing_metrics.h"
#include "storage/replacement/replacement_storage.h"
#include "loggers/replacement/replacement_logger.h"
#include "storage/removal/removal_storage.h"
#include "loggers/removal/removal_logger.h"

// Helper function
std::vector<TaskEnv*> initializeTasks(TPG& tpg);

int main(int argc, char** argv) {
   mpi::environment env(argc, argv);
   mpi::communicator world;
   TPG tpg;
   tpg.state_["world_rank"] = world.rank();
   tpg.SetParams(argc, argv);

   // APIClient* apiClient = nullptr;

   // if (tpg.GetParam<int>("track_experiments")) {
   //    // Only instantiate APIClient if trackExperiment is true
   //    apiClient = new APIClient(getenv("COMET_API_KEY"),
   //                              tpg.GetParam<std::string>("experiment_key"));

   //    // Track run parameters
   //    cout << "Tracking experiment parameters" << endl;
   //    for (auto& param : tpg.params_) {
   //       std::string value;

   //       if (param.second.type() == typeid(int)) {
   //          value = std::to_string(std::any_cast<int>(param.second));
   //       } else if (param.second.type() == typeid(double)) {
   //          value = std::to_string(std::any_cast<double>(param.second));
   //       } else {
   //          value = std::any_cast<std::string>(param.second);
   //       }

   //       apiClient->LogParameter(param.first, value);
   //    }

   //    // Add experiment tracking to TPG
   //    tpg.InitExperimentTracking(apiClient);
   // }

   ostringstream os;  // logging

   // Read task sets from parameters and create environments
   vector<TaskEnv*> tasks = initializeTasks(tpg);

   // Create task indices vector
   vector<int> taskIndices;
   for (int i = 0; i < (int)tasks.size(); i++)
      taskIndices.push_back(i);

   // Read number of inpts per task from parameters
   stringstream ss(tpg.GetParam<string>("n_input"));
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

   int seed_tpg = tpg.GetParam<int>("seed_tpg");
   int pid = tpg.GetParam<int>("pid");

   if (tpg.GetParam<int>("replay") == 0) {
      // Initialize logger classes
      SelectionStorage::instance().init(seed_tpg, pid);
      SelectionLogger selectionLogger;
      selectionLogger.init();
      TimingStorage::instance().init(seed_tpg, pid);
      TimingLogger timingLogger;
      timingLogger.init();
      ReplacementStorage::instance().init(seed_tpg, pid);
      ReplacementLogger replacementLogger;
      replacementLogger.init();
      RemovalStorage::instance().init(seed_tpg, pid);
      RemovalLogger removalLogger;
      removalLogger.init();
   };

   if (world.rank() == 0) {  // Master Process
      string my_string = "MAIN";

      // Initialization //////////////////////////////////////////////////////
      if (tpg.GetParam<int>("start_from_checkpoint")) {
         tpg.ReadCheckpoint(tpg.GetParam<int>("checkpoint_in_t"),
                            tpg.GetParam<int>("checkpoint_in_phase"), false,
                            "");                                   
      } else {
         tpg.InitTeams();
      }

      ExperimentRunner* runner = nullptr;

      // Main training loop.
      tpg.state_["phase"] = _TRAIN_PHASE;
      if (tpg.GetParam<int>("replay")) {
         runner = new ReplayRunner(tpg, tasks);
      } else {
         runner = new TrainingRunner(tpg, tasks, world, taskIndices);
      }

      runner->run();

      for (int ev = 1; ev <= world.size() - 1; ev++) {
         string d = "done";
         delete runner;
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

vector<TaskEnv*> initializeTasks(TPG& tpg) {
   vector<TaskEnv*> tasks;
   stringstream ss(tpg.GetParam<std::string>("active_tasks"));
   string taskName;
   
   while (std::getline(ss, taskName, ',')) {
       TaskEnv* task = TaskEnvFactory::createTask(taskName, tpg.params_);
       if (!task) {
           std::cerr << "Unrecognized task: " << taskName << std::endl;
           exit(1);
       }

       tasks.push_back(task);

      // Configuration specific to this application
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
   return tasks;
}

