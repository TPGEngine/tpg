#include "replay_runner.h"
#include "streaming/signaling_client.h"
#include "streaming/pipeline.h"
#include <iostream>
#include <chrono>
#include <thread>

ReplayRunner::ReplayRunner(TPG& tpg, std::vector<TaskEnv*>& tasks)
    : tpg_(tpg), tasks_(tasks), main_loop_(nullptr) {
    std::cout << "[GMainLoop] ReplayRunner constructor called" << std::endl;
}

ReplayRunner::~ReplayRunner() {
  if (main_loop_) {
    std::cout << "[GMainLoop] Cleaning up GMainLoop" << std::endl;
    g_main_loop_unref(main_loop_);
    std::cout << "[GMainLoop] GMainLoop cleaned up successfully" << std::endl;
  }
}

void ReplayRunner::performReplay() {
  std::cout << "[GMainLoop] Starting replay in thread " << std::this_thread::get_id() << std::endl;
  
  // Set appropriate phase for replay
  tpg_.state_["phase"] = _TEST_PHASE;
  tpg_.state_["active_task"] = tpg_.GetParam<int>("task_to_replay");

  // Process parameters and evaluate the specific team
  tpg_.ProcessParams();
  tpg_.MarkEffectiveCode();

  std::cout << "[GMainLoop] Calling replayer function" << std::endl;
  // Call the replay function that animates or evaluates an individual.
  replayer(tpg_, tasks_);
  std::cout << "[GMainLoop] Replayer function completed" << std::endl;

  // After replay is done, quit the main loop if it's running
  if (main_loop_ && g_main_loop_is_running(main_loop_)) {
    std::cout << "[GMainLoop] Quitting main loop" << std::endl;
    g_main_loop_quit(main_loop_);
    std::cout << "[GMainLoop] Main loop quit signal sent" << std::endl;
  }
}

void ReplayRunner::initialization() {
  std::cout << "[GMainLoop] Starting initialization in thread " << std::this_thread::get_id() << std::endl;
  
  // Read checkpoint data
  std::cout << "[GMainLoop] Reading checkpoint data" << std::endl;
  tpg_.ReadCheckpoint(tpg_.GetParam<int>("checkpoint_in_t"),
                      tpg_.GetParam<int>("checkpoint_in_phase"),
                      false, "");

  // Initialize the WebSocket signaling connection.
  std::cout << "[GMainLoop] Initializing WebSocket connection" << std::endl;
  auto& signalingClient =
      WebSocketClient::getInstance("localhost", "8000", "/ws/signaling");

  // Set a message handler to process incoming signaling messages.
  signalingClient.setMessageHandler([](const std::string &msg) {
    std::cout << "[Signaling] Received: " << msg << std::endl;
    // Forward the message to GStreamerPipeline for processing
    GStreamerPipeline::getInstance().handleSignalingMessage(msg);
  });

  // Connect the signaling client to the FastAPI WebSocket signaling server.
  std::cout << "[GMainLoop] Connecting WebSocket client" << std::endl;
  signalingClient.connect();

  // Create and start the GMainLoop
  std::cout << "[GMainLoop] Creating GMainLoop" << std::endl;
  main_loop_ = g_main_loop_new(NULL, FALSE);
  if (!main_loop_) {
    std::cerr << "[GMainLoop] Failed to create GMainLoop" << std::endl;
    return;
  }
  std::cout << "[GMainLoop] GMainLoop created successfully" << std::endl;
}

void ReplayRunner::run() {
  std::cout << "[GMainLoop] Starting run in thread " << std::this_thread::get_id() << std::endl;
  initialization();
  
  // Start the replay in a separate thread
  std::cout << "[GMainLoop] Creating replay thread" << std::endl;
  std::thread replay_thread([this]() {
    performReplay();
  });

  // Run the GMainLoop in the main thread
  if (main_loop_) {
    std::cout << "[GMainLoop] Starting GMainLoop in main thread" << std::endl;
    g_main_loop_run(main_loop_);
    std::cout << "[GMainLoop] GMainLoop has stopped" << std::endl;
  }

  // Wait for the replay thread to finish
  std::cout << "[GMainLoop] Waiting for replay thread to finish" << std::endl;
  if (replay_thread.joinable()) {
    replay_thread.join();
    std::cout << "[GMainLoop] Replay thread joined successfully" << std::endl;
  }
  std::cout << "[GMainLoop] Run completed" << std::endl;
}
