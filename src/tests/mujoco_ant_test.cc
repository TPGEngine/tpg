#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "../src/cpp/environments/mujoco/Mujoco_Ant_v4.h"
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

TEST_CASE("Mujoco_Ant_v4 Initialization", "[init]") {
    std::unordered_map<std::string, std::any> params = createDefaultParams();
    INFO("Initializing Environment");
    Mujoco_Ant_v4 ant(params);

    INFO("Testing Initialization");
    REQUIRE(ant.n_eval_train_ == 10);
    REQUIRE(ant.n_eval_validation_ == 5);
    REQUIRE(ant.n_eval_test_ == 3);
    REQUIRE(ant.max_step_ == 100);
}

TEST_CASE("Mujoco_Ant_v4 Terminal Condition", "[terminal]") {
    std::unordered_map<std::string, std::any> params = createDefaultParams();
    Mujoco_Ant_v4 ant(params);

    INFO("Testing Terminal Step Count");
    ant.step_ = 200;
    REQUIRE(ant.terminal() == true);
}

TEST_CASE("Mujoco_Ant_v4 Healthy Reward", "[healthy_reward]") {
    std::unordered_map<std::string, std::any> params = createDefaultParams();
    Mujoco_Ant_v4 ant(params);

    REQUIRE(ant.healthy_reward() == ant.healthy_reward_);
}

TEST_CASE("Mujoco_Ant_v4 Control Cost", "[control_cost]") {
    std::unordered_map<std::string, std::any> params = createDefaultParams();
    Mujoco_Ant_v4 ant(params);

    std::vector<double> action = {0.1, -0.1, 0.2, 0.3};
    double expected_cost = ant.control_cost_weight_ * (0.1 * 0.1 + (-0.1) * (-0.1) + 0.2 * 0.2 + 0.3 * 0.3);
    REQUIRE(ant.control_cost(action) == Approx(expected_cost));
}

TEST_CASE("Mujoco_Ant_v4 Contact Cost", "[contact_cost]") {
    std::unordered_map<std::string, std::any> params = createDefaultParams();
    Mujoco_Ant_v4 ant(params);

    REQUIRE(ant.contact_cost() >= 0.0);
}

TEST_CASE("Mujoco_Ant_v4 Is Healthy", "[is_healthy]") {
    std::unordered_map<std::string, std::any> params = createDefaultParams();
    Mujoco_Ant_v4 ant(params);

    ant.d_->qpos[2] = 0.5; // Within healthy z range
    REQUIRE(ant.is_healthy() == true);

    ant.d_->qpos[2] = 0.1; // Below healthy range
    REQUIRE(ant.is_healthy() == false);
}

TEST_CASE("Mujoco_Ant_v4 Simulation Step", "[sim_step]") {
    std::unordered_map<std::string, std::any> params = createDefaultParams();
    Mujoco_Ant_v4 ant(params);

    std::vector<double> action = {0.1, -0.1, 0.2, 0.3}; // Example action input
    auto result = ant.sim_step(action);
    
    REQUIRE(std::isfinite(result.r1));
    REQUIRE(ant.step_ == 1); // Ensure step count is incremented
}

TEST_CASE("Mujoco_Ant_v4 Get Observation", "[get_obs]") {
    std::unordered_map<std::string, std::any> params = createDefaultParams();
    Mujoco_Ant_v4 ant(params);

    REQUIRE(ant.m_ != nullptr);
    REQUIRE(ant.d_ != nullptr);
    REQUIRE(ant.d_->qpos != nullptr);
    REQUIRE(ant.d_->qvel != nullptr);

    std::vector<double> obs(ant.obs_size_, 1.0);
    ant.get_obs(obs);

    std::vector<double> zero_obs(obs.size(), 0.0);
    REQUIRE(obs != zero_obs);
}

TEST_CASE("Mujoco_Ant_v4 Reset Function", "[reset]") {
    std::unordered_map<std::string, std::any> params = createDefaultParams();
    Mujoco_Ant_v4 ant(params);
    std::mt19937 rng(1234);

    ant.step_ = 50; 

    std::vector<double> qpos = {0.5, 0.8, -0.3};
    std::vector<double> qvel = {0.1, -0.05, 0.05};
    ant.set_state(qpos, qvel);

    std::vector<double> obs(ant.obs_size_, 1.0);
    ant.get_obs(obs);
    
    ant.reset(rng);

    REQUIRE(ant.step_ == 0);

    for (size_t i = 0; i < ant.state_.size(); i++) {
        REQUIRE(ant.state_[i] == Catch::Approx(0.0).margin(1e-2));
    }
}
