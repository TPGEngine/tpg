#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "../src/environments/mujoco/Mujoco_Half_Cheetah_v4.h"
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

TEST_CASE("Mujoco_Half_Cheetah_v4 Initialization", "[init]") {
    std::unordered_map<std::string, std::any> params = createDefaultParams();
    INFO("Initializing Environment");
    Mujoco_Half_Cheetah_v4 half_cheetah(params);

    INFO("Testing Initialization");
    REQUIRE(half_cheetah.n_eval_train_ == 10);
    REQUIRE(half_cheetah.n_eval_validation_ == 5);
    REQUIRE(half_cheetah.n_eval_test_ == 3);
    REQUIRE(half_cheetah.max_step_ == 100);
}

TEST_CASE("Mujoco_Half_Cheetah_v4 Terminal Condition", "[terminal]") {
    std::unordered_map<std::string, std::any> params = createDefaultParams();
    Mujoco_Half_Cheetah_v4 half_cheetah(params);

    INFO("Testing Terminal Step Count");
    half_cheetah.step_ = 200;
    REQUIRE(half_cheetah.terminal() == true);
}

TEST_CASE("Mujoco_Half_Cheetah_v4 Control Cost", "[control_cost]") {
    std::unordered_map<std::string, std::any> params = createDefaultParams();
    Mujoco_Half_Cheetah_v4 half_cheetah(params);

    std::vector<double> action = {0.1, -0.1, 0.2};
    double expected_cost = half_cheetah.control_cost_weight_ * (0.1 * 0.1 + (-0.1) * (-0.1) + 0.2 * 0.2);
    REQUIRE(half_cheetah.control_cost(action) == Approx(expected_cost));
}

TEST_CASE("Mujoco_Half_Cheetah_v4 Simulation Step", "[sim_step]") {
    std::unordered_map<std::string, std::any> params = createDefaultParams();
    Mujoco_Half_Cheetah_v4 half_cheetah(params);

    std::vector<double> action = {0.1, -0.1, 0.2}; // Example action input
    auto result = half_cheetah.sim_step(action);
    
    REQUIRE(std::isfinite(result.r1));
    REQUIRE(half_cheetah.step_ == 1); // Ensure step count is incremented
}

TEST_CASE("Mujoco_Half_Cheetah_v4 Reset Function", "[reset]") {
    std::unordered_map<std::string, std::any> params = createDefaultParams();
    Mujoco_Half_Cheetah_v4 half_cheetah(params);
    std::mt19937 rng(1234);

    half_cheetah.step_ = 50; 

    std::vector<double> qpos = {0.5, 0.8, -0.3};
    std::vector<double> qvel = {0.1, -0.05, 0.05};
    half_cheetah.set_state(qpos, qvel);

    std::vector<double> obs(half_cheetah.obs_size_, 1.0);
    half_cheetah.get_obs(obs);
    
    half_cheetah.reset(rng);

    REQUIRE(half_cheetah.step_ == 0);

    for (size_t i = 0; i < half_cheetah.state_.size(); i++) {
        REQUIRE(half_cheetah.state_[i] == Catch::Approx(0.0).margin(1e-2));
    }
}