// TrainingRunner.h
#ifndef TRAINING_RUNNER_H
#define TRAINING_RUNNER_H

#include "experiment_runner.h"

#define PRINT_MOD 1
#define CHECKPOINT_MOD 1000000
#define PRINT_MOD 1
#define NUM_POINT_AUX_DOUBLE 3
#define NUM_POINT_AUX_INT 4
#define MODES_T 1000000000

class TaskEnv;

class TrainingRunner : public ExperimentRunner {
 public:
  TrainingRunner(TPG& tpg, std::vector<TaskEnv*>& tasks, boost::mpi::communicator& world, std::vector<int>& taskIndices);
  void run() override;

 private:
  void initialization();
  void trainingLoop();
  // void logGenerationMetrics();

  TPG& tpg_;
  std::vector<TaskEnv*>& tasks_;
  boost::mpi::communicator& world_;
  std::vector<int>& taskIndices_;
};

#endif  // TRAINING_RUNNER_H
