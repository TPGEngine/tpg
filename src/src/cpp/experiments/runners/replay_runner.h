// ReplayRunner.h
#ifndef REPLAY_RUNNER_H
#define REPLAY_RUNNER_H

#include "experiment_runner.h"

class TaskEnv;

class ReplayRunner : public ExperimentRunner {
 public:
  ReplayRunner(TPG& tpg, std::vector<TaskEnv*>& tasks);
  void run() override;

 private:
  void initialization();
  void performReplay();

  TPG& tpg_;
  std::vector<TaskEnv*>& tasks_;
};

#endif
