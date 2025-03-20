#include "replay_runner.h"
#include "streaming/signaling_client.h"
#include <iostream>

ReplayRunner::ReplayRunner(TPG& tpg, std::vector<TaskEnv*>& tasks)
    : tpg_(tpg), tasks_(tasks) {}

void ReplayRunner::performReplay() {
  // Set appropriate phase for replay
  tpg_.state_["phase"] = _TEST_PHASE;
  tpg_.state_["active_task"] = tpg_.GetParam<int>("task_to_replay");

  // Process parameters and evaluate the specific team
  tpg_.ProcessParams();
  tpg_.MarkEffectiveCode();

  // Optionally, you can add any signaling-related logic here as needed.
  // Then, call the replay function that animates or evaluates an individual.
  replayer(tpg_, tasks_);
}

void ReplayRunner::initialization() {
  // Read checkpoint data
  tpg_.ReadCheckpoint(tpg_.GetParam<int>("checkpoint_in_t"),
                      tpg_.GetParam<int>("checkpoint_in_phase"),
                      false, "");

  // Initialize the WebSocket signaling connection.
  // Get the singleton instance (optionally, you can adjust the host, port, and target).
  auto& signalingClient =
      WebSocketClient::getInstance("localhost", "8000", "/ws/signaling");

  // Set a message handler to process incoming signaling messages.
  signalingClient.setMessageHandler([](const std::string &msg) {
    std::cout << "[Signaling] Received: " << msg << std::endl;
    // Process signaling messages (SDP, ICE candidates, etc.) here as needed.
  });

  // Connect the signaling client to the FastAPI WebSocket signaling server.
  signalingClient.connect();
}

void ReplayRunner::run() {
  initialization();
  performReplay();
}
