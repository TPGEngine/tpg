#include "replay_runner.h"

ReplayRunner::ReplayRunner(TPG& tpg, std::vector<TaskEnv*>& tasks)
    : tpg_(tpg), tasks_(tasks) {}

void ReplayRunner::performReplay() {
  // Set appropriate phase for replay
  tpg_.state_["phase"] = _TEST_PHASE;
  tpg_.state_["active_task"] = tpg_.GetParam<int>("task_to_replay");
  // Process params and evaluate the specific team
  tpg_.ProcessParams();
  tpg_.MarkEffectiveCode();
  // Call the replay function that animates or evaluates an individual
  replayer(tpg_, tasks_);
}

void ReplayRunner::initialization() {
    tpg_.ReadCheckpoint(tpg_.GetParam<int>("checkpoint_in_t"),
                        tpg_.GetParam<int>("checkpoint_in_phase"), false,
                        "");                                   

}

void ReplayRunner::run() {
  initialization();
  performReplay();
}
