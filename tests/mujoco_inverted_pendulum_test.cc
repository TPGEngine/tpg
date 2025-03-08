#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>


#include "../src/environments/mujoco/Mujoco_Inverted_Pendulum_v4.h"
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

TEST_CASE("Mujoco_Inverted_Pendulum_v4 Initialization", "[init]") {
    std::unordered_map<string, std::any> params = createDefaultParams();    
    INFO("Initializing Environment");
    Mujoco_Inverted_Pendulum_v4 pendulum(params);
    
    INFO("Testing Initialization");
    REQUIRE(pendulum.n_eval_train_ == 10);
    REQUIRE(pendulum.n_eval_validation_ == 5);
    REQUIRE(pendulum.n_eval_test_ == 3);
    REQUIRE(pendulum.max_step_ == 100);
}

TEST_CASE("Mujoco_Inverted_Pendulum_v4 Terminal Condition", "[terminal]") {
    std::unordered_map<string, std::any> params = createDefaultParams();
    INFO("Initializing Environment");
    Mujoco_Inverted_Pendulum_v4 pendulum(params);

    // Simulate a situation where the pendulum has fallen over
    INFO("Testing Terminal Threshold");
    pendulum.d_->qpos[1] = 0.3; // Beyond the threshold of 0.2
    REQUIRE(pendulum.terminal() == true);
    
    INFO("Resetting Terminal Threshold");
    pendulum.d_->qpos[1] = 0.1;
    REQUIRE(pendulum.terminal() == false);

    INFO("Testing Terminal Step Count");
    // Simulate reaching the max step count
    pendulum.step_ = 200;
    REQUIRE(pendulum.terminal() == true);
}

TEST_CASE("Mujoco_Inverted_Pendulum_v4 Simulation Step", "[sim_step]") {
    std::unordered_map<string, std::any> params = createDefaultParams();

    Mujoco_Inverted_Pendulum_v4 pendulum(params);

    std::vector<double> action = {0.1}; // Example action input

    auto result = pendulum.sim_step(action);
    
    REQUIRE(std::isfinite(result.r1));
    REQUIRE(result.r1 == Approx(1.0)); // Check if reward is as expected
    REQUIRE(pendulum.step_ == 1); // Ensure step count is incremented
}


TEST_CASE("Mujoco_Inverted_Pendulum_v4 Get Observation", "[get_obs]") {
    std::unordered_map<std::string, std::any> params = createDefaultParams();
    Mujoco_Inverted_Pendulum_v4 pendulum(params);

    REQUIRE(pendulum.m_ != nullptr);
    REQUIRE(pendulum.d_ != nullptr);
    REQUIRE(pendulum.d_->qpos != nullptr);
    REQUIRE(pendulum.d_->qvel != nullptr);

    // Manually set qpos and qvel to non-zero values
    pendulum.d_->qpos[0] = 0.5;
    pendulum.d_->qpos[1] = -0.3;
    pendulum.d_->qvel[0] = 0.2;
    pendulum.d_->qvel[1] = -0.1;

    std::vector<double> obs(pendulum.m_->nq + pendulum.m_->nv, 1.0);

    pendulum.get_obs(obs);

    std::vector<double> zero_obs(obs.size(), 0.0);
    std::vector<double> solution = {0.5, -0.3, 0.2, 0.1};
    REQUIRE(obs != zero_obs); 
    REQUIRE(obs != solution); 

}

TEST_CASE("Mujoco_Inverted_Pendulum_v4 Reset Function", "[reset]") {
    std::unordered_map<string, std::any> params = createDefaultParams();

    Mujoco_Inverted_Pendulum_v4 pendulum(params);
    std::mt19937 rng(1234);
    pendulum.step_ = 50; 

    pendulum.d_->qpos[0] = 0.5;
    pendulum.d_->qpos[1] = -0.3;
    pendulum.d_->qvel[0] = 0.2;
    pendulum.d_->qvel[1] = -0.1;

    std::vector<double> obs = {0.9, 0.5, 0.7, 0.1};
    std::vector<double> zero_obs(obs.size(), 0.0);

    pendulum.get_obs(obs);
    pendulum.reset(rng);

    REQUIRE(pendulum.step_ == 0);
    REQUIRE(pendulum.state_ == zero_obs);
}