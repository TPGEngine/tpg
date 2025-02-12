#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "../src/cpp/environments/mujoco/Mujoco_Hopper_v4.h"
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

    const char* tpg_env = std::getenv("TPG");
    if (tpg_env == nullptr) {
        throw std::runtime_error("TPG environment variable is not set.");
    }
    
    std::unordered_map<std::string, std::any> params;
    params["mj_n_eval_train"] = 10;
    params["mj_n_eval_validation"] = 5;
    params["mj_n_eval_test"] = 3;
    params["mj_max_timestep"] = 100;
    params["mj_model_path"] = std::string(tpg_env) + "/datasets/mujoco_models/";
    params["mj_reward_control_weight"] = 0.0;
    return params;
}

TEST_CASE("Mujoco_Hopper_v4 Initialization", "[init]") {
    std::unordered_map<std::string, std::any> params = createDefaultParams();
    INFO("Initializing Environment");
    Mujoco_Hopper_v4 hopper(params);

    INFO("Testing Initialization");
    REQUIRE(hopper.n_eval_train_ == 10);
    REQUIRE(hopper.n_eval_validation_ == 5);
    REQUIRE(hopper.n_eval_test_ == 3);
    REQUIRE(hopper.max_step_ == 100);
}

TEST_CASE("Mujoco_Hopper_v4 Terminal Condition", "[terminal]") {
    std::unordered_map<std::string, std::any> params = createDefaultParams();
    Mujoco_Hopper_v4 hopper(params);

    INFO("Testing Terminal Threshold");
    hopper.d_->qpos[1] = 0.6; // Below the healthy z range of 0.7
    REQUIRE(hopper.terminal() == true);
    
    INFO("Resetting Terminal Threshold");
    hopper.d_->qpos[1] = 0.8;
    REQUIRE(hopper.terminal() == false);

    INFO("Testing Terminal Step Count");
    hopper.step_ = 200;
    REQUIRE(hopper.terminal() == true);
}

TEST_CASE("Mujoco_Hopper_v4 Healthy Reward", "[healthy_reward]") {
    std::unordered_map<std::string, std::any> params = createDefaultParams();
    Mujoco_Hopper_v4 hopper(params);

    REQUIRE(hopper.healthy_reward() == hopper.healthy_reward_);
}

TEST_CASE("Mujoco_Hopper_v4 Control Cost", "[control_cost]") {
    std::unordered_map<std::string, std::any> params = createDefaultParams();
    Mujoco_Hopper_v4 hopper(params);

    std::vector<double> action = {0.1, -0.1, 0.2};
    double expected_cost = hopper.control_cost_weight_ * (0.1 * 0.1 + (-0.1) * (-0.1) + 0.2 * 0.2);
    REQUIRE(hopper.control_cost(action) == Approx(expected_cost));
}

TEST_CASE("Mujoco_Hopper_v4 Is Healthy", "[is_healthy]") {
    std::unordered_map<std::string, std::any> params = createDefaultParams();
    Mujoco_Hopper_v4 hopper(params);

    hopper.d_->qpos[1] = 0.8; // Within healthy range
    hopper.d_->qpos[2] = 0.0; // Neutral angle
    REQUIRE(hopper.is_healthy() == true);

    hopper.d_->qpos[1] = 0.6; // Below healthy z range
    REQUIRE(hopper.is_healthy() == false);

    hopper.d_->qpos[1] = 0.8; // Restore healthy z
    hopper.d_->qpos[2] = 0.3; // Outside healthy angle range
    REQUIRE(hopper.is_healthy() == false);
}

TEST_CASE("Mujoco_Hopper_v4 Simulation Step", "[sim_step]") {
    std::unordered_map<std::string, std::any> params = createDefaultParams();
    Mujoco_Hopper_v4 hopper(params);

    std::vector<double> action = {0.1, -0.1, 0.2}; // Example action input
    auto result = hopper.sim_step(action);
    
    REQUIRE(std::isfinite(result.r1));
    REQUIRE(hopper.step_ == 1); // Ensure step count is incremented
}

TEST_CASE("Mujoco_Hopper_v4 Get Observation", "[get_obs]") {
    std::unordered_map<std::string, std::any> params = createDefaultParams();
    Mujoco_Hopper_v4 hopper(params);

    REQUIRE(hopper.m_ != nullptr);
    REQUIRE(hopper.d_ != nullptr);
    REQUIRE(hopper.d_->qpos != nullptr);
    REQUIRE(hopper.d_->qvel != nullptr);

    // Manually set qpos and qvel to non-zero values
    hopper.d_->qpos[0] = 0.5;
    hopper.d_->qpos[1] = 0.8;
    hopper.d_->qpos[2] = -0.3;
    hopper.d_->qvel[0] = 0.1;
    hopper.d_->qvel[1] = -0.05;
    hopper.d_->qvel[2] = 0.05;

    std::vector<double> obs(hopper.obs_size_, 1.0);
    hopper.get_obs(obs);

    std::vector<double> zero_obs(obs.size(), 0.0);
    REQUIRE(obs != zero_obs);
}

TEST_CASE("Mujoco_Hopper_v4 Reset Function", "[reset]") {
    std::unordered_map<std::string, std::any> params = createDefaultParams();
    Mujoco_Hopper_v4 hopper(params);
    std::mt19937 rng(1234);

    hopper.step_ = 50; 

    hopper.d_->qpos[0] = 0.5;
    hopper.d_->qpos[1] = -0.3;
    hopper.d_->qpos[2] = 0.2;
    hopper.d_->qvel[0] = 0.1;
    hopper.d_->qvel[1] = -0.05;
    hopper.d_->qvel[2] = 0.05;

    std::vector<double> obs = {0.9, 0.5, 0.7, 0.1, 0.5, 0.9, 0.4, 0.2, 0.4, 0.5, 0.2}; // random obs values
    std::vector<double> zero_obs(obs.size(), 0.0);
    
    hopper.reset(rng);

    REQUIRE(hopper.step_ == 0);
    REQUIRE(hopper.state_ == zero_obs);
}

