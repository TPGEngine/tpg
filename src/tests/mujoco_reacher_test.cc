#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "../src/cpp/environments/mujoco/Mujoco_Reacher_v4.h"
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
    params["mj_model_path"] = std::string("$TPG/datasets/mujoco_models/reacher.xml");
    return params;
}

TEST_CASE("Mujoco_Reacher_v4 Initialization", "[init]") {
    std::unordered_map<std::string, std::any> params = createDefaultParams();
    INFO("Initializing Environment");
    Mujoco_Reacher_v4 reacher(params);

    INFO("Testing Initialization");
    REQUIRE(reacher.n_eval_train_ == 10);
    REQUIRE(reacher.n_eval_validation_ == 5);
    REQUIRE(reacher.n_eval_test_ == 3);
    REQUIRE(reacher.max_step_ == 100);
}

TEST_CASE("Mujoco_Reacher_v4 Terminal Condition", "[terminal]") {
    std::unordered_map<std::string, std::any> params = createDefaultParams();
    Mujoco_Reacher_v4 reacher(params);

    INFO("Testing Terminal Step Count");
    reacher.step_ = 200;
    REQUIRE(reacher.terminal() == true);
}

TEST_CASE("Mujoco_Reacher_v4 Control Cost", "[control_cost]") {
    std::unordered_map<std::string, std::any> params = createDefaultParams();
    Mujoco_Reacher_v4 reacher(params);

    std::vector<double> action = {0.1, -0.1};
    double expected_cost = reacher.reward_control_weight * (0.1 * 0.1 + (-0.1) * (-0.1));
    REQUIRE(reacher.control_cost(action) == Approx(expected_cost));
}

TEST_CASE("Mujoco_Reacher_v4 Get Distance", "[get_dist]") {
    std::unordered_map<std::string, std::any> params = createDefaultParams();
    Mujoco_Reacher_v4 reacher(params);

    std::vector<double> dist = reacher.get_dist();
    REQUIRE(dist.size() == 2);
}

TEST_CASE("Mujoco_Reacher_v4 Simulation Step", "[sim_step]") {
    std::unordered_map<std::string, std::any> params = createDefaultParams();
    Mujoco_Reacher_v4 reacher(params);

    std::vector<double> action = {0.1, -0.1}; // Example action input
    auto result = reacher.sim_step(action);
    
    REQUIRE(std::isfinite(result.r1));
    REQUIRE(reacher.step_ == 1); // Ensure step count is incremented
}

TEST_CASE("Mujoco_Reacher_v4 Get Observation", "[get_obs]") {
    std::unordered_map<std::string, std::any> params = createDefaultParams();
    Mujoco_Reacher_v4 reacher(params);

    REQUIRE(reacher.m_ != nullptr);
    REQUIRE(reacher.d_ != nullptr);
    REQUIRE(reacher.d_->qpos != nullptr);
    REQUIRE(reacher.d_->qvel != nullptr);

    std::vector<double> obs(reacher.obs_size_, 1.0);
    reacher.get_obs(obs);

    std::vector<double> zero_obs(obs.size(), 0.0);
    REQUIRE(obs != zero_obs);
}

TEST_CASE("Mujoco_Reacher_v4 Reset Function", "[reset]") {
    std::unordered_map<std::string, std::any> params = createDefaultParams();
    Mujoco_Reacher_v4 reacher(params);
    std::mt19937 rng(1234);

    reacher.step_ = 50; 

    std::vector<double> qpos = {0.5, 0.8, -0.3};
    std::vector<double> qvel = {0.1, -0.05, 0.05};
    reacher.set_state(qpos, qvel);

    std::vector<double> obs(reacher.obs_size_, 1.0);
    reacher.get_obs(obs);
    
    reacher.reset(rng);

    REQUIRE(reacher.step_ == 0);

    for (size_t i = 0; i < reacher.state_.size(); i++) {
        REQUIRE(reacher.state_[i] == Catch::Approx(0.0).margin(1e-2));
    }
}
