#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "../src/cpp/environments/mujoco/Mujoco_Inverted_Double_Pendulum_v4.h"
#include <random>
#include <TPG.h>
#include <instruction.h>

using Catch::Approx;

// Override mjr_freeContext in the test environment
extern "C" void mjr_freeContext(mjrContext* con) {
    if (con == nullptr || con->ntexture == 0) {
        std::cout << "Skipping mjr_freeContext to prevent segfault" << std::endl;
        return;
    }
    std::cout << "Mocking mjr_freeContext" << std::endl;
}

// Helper function to create default parameters
std::unordered_map<std::string, std::any> createDefaultParams() {
    std::unordered_map<std::string, std::any> params;
    params["mj_n_eval_train"] = 10;
    params["mj_n_eval_validation"] = 5;
    params["mj_n_eval_test"] = 3;
    params["mj_max_timestep"] = 100;
    params["mj_model_path"] = std::string("$TPG/datasets/mujoco_models/inverted_double_pendulum.xml");
    return params;
}

TEST_CASE("Mujoco_Inverted_Double_Pendulum_v4 Initialization", "[init]") {
    std::unordered_map<std::string, std::any> params = createDefaultParams();
    INFO("Initializing Environment");
    Mujoco_Inverted_Double_Pendulum_v4 pendulum(params);

    INFO("Testing Initialization");
    REQUIRE(pendulum.n_eval_train_ == 10);
    REQUIRE(pendulum.n_eval_validation_ == 5);
    REQUIRE(pendulum.n_eval_test_ == 3);
    REQUIRE(pendulum.max_step_ == 100);
}

TEST_CASE("Mujoco_Inverted_Double_Pendulum_v4 Terminal Condition", "[terminal]") {
    std::unordered_map<std::string, std::any> params = createDefaultParams();
    Mujoco_Inverted_Double_Pendulum_v4 pendulum(params);

    // Simulate a situation where the pendulum has fallen over
    INFO("Testing Terminal Threshold");
    pendulum.d_->site_xpos[2] = 0.9; // Below the threshold of 1.0
    REQUIRE(pendulum.terminal() == true);
    
    INFO("Resetting Terminal Threshold");
    pendulum.d_->site_xpos[2] = 1.5;
    REQUIRE(pendulum.terminal() == false);

    INFO("Testing Terminal Step Count");
    // Simulate reaching the max step count
    pendulum.step_ = 200;
    REQUIRE(pendulum.terminal() == true);
}

TEST_CASE("Mujoco_Inverted_Double_Pendulum_v4 Simulation Step", "[sim_step]") {
    std::unordered_map<std::string, std::any> params = createDefaultParams();
    Mujoco_Inverted_Double_Pendulum_v4 pendulum(params);

    std::vector<double> action = {0.1}; // Example action input

    auto result = pendulum.sim_step(action);
    
    REQUIRE(result.r1 >= 0.0); // Reward should be non-negative
    REQUIRE(pendulum.step_ == 1); // Ensure step count is incremented
}

TEST_CASE("Mujoco_Inverted_Double_Pendulum_v4 Get Observation", "[get_obs]") {
    std::unordered_map<std::string, std::any> params = createDefaultParams();
    Mujoco_Inverted_Double_Pendulum_v4 pendulum(params);

    REQUIRE(pendulum.m_ != nullptr);
    REQUIRE(pendulum.d_ != nullptr);
    REQUIRE(pendulum.d_->qpos != nullptr);
    REQUIRE(pendulum.d_->qvel != nullptr);

    // Manually set qpos and qvel to non-zero values
    pendulum.d_->qpos[0] = 0.5;
    pendulum.d_->qpos[1] = 0.3;
    pendulum.d_->qpos[2] = -0.2;
    pendulum.d_->qvel[0] = 0.1;
    pendulum.d_->qvel[1] = -0.1;
    pendulum.d_->qvel[2] = 0.05;

    std::vector<double> obs(pendulum.obs_size_, 1.0);
    pendulum.get_obs(obs);

    std::vector<double> zero_obs(obs.size(), 0.0);
    REQUIRE(obs != zero_obs);
}

TEST_CASE("Mujoco_Inverted_Double_Pendulum_v4 Reset Function", "[reset]") {
    std::unordered_map<std::string, std::any> params = createDefaultParams();
    Mujoco_Inverted_Double_Pendulum_v4 pendulum(params);
    std::mt19937 rng(1234);

    pendulum.step_ = 50; 

    pendulum.d_->qpos[0] = 0.5;
    pendulum.d_->qpos[1] = -0.3;
    pendulum.d_->qpos[2] = 0.2;
    pendulum.d_->qvel[0] = 0.1;
    pendulum.d_->qvel[1] = -0.05;
    pendulum.d_->qvel[2] = 0.05;

    std::vector<double> obs = {0.9, 0.5, 0.7, 0.1, 0.5, 0.9, 0.4, 0.2, 0.4, 0.5, 0.2}; // random obs values
    std::vector<double> zero_obs(obs.size(), 0.0);

    pendulum.get_obs(obs);
    pendulum.reset(rng);

    REQUIRE(pendulum.step_ == 0);
    REQUIRE(pendulum.state_ == zero_obs);
    REQUIRE(pendulum.terminalState == false);
}
