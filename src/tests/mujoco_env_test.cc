#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "mock_mujoco_env.h"

#include <string>  
#include <vector>
#include <random>
#include <iostream>

using Catch::Approx;

// MuJoCo Initialize Simulation Test
TEST_CASE("Mujoco Environment - Simulation Initialization", "[MujocoEnv]") {
    MockMujocoEnv env("../datasets/mujoco_models/inverted_pendulum.xml");

    REQUIRE_NOTHROW(env.initialize_simulation());
    REQUIRE(env.m_ != nullptr);
    REQUIRE(env.d_ != nullptr);
}

// MuJoCo Set State Test
TEST_CASE("Mujoco Environment - Set State", "[MujocoEnv]") {
    MockMujocoEnv env("../datasets/mujoco_models/inverted_pendulum.xml");
    env.initialize_simulation();

    std::vector<double> qpos(env.m_->nq, 0.5);
    std::vector<double> qvel(env.m_->nv, 0.1);

    REQUIRE_NOTHROW(env.set_state(qpos, qvel));

    for (int i = 0; i < env.m_->nq; i++) {
        CHECK(env.d_->qpos[i] == Approx(0.5));
    }
    for (int i = 0; i < env.m_->nv; i++) {
        CHECK(env.d_->qvel[i] == Approx(0.1));
    }
}

// MuJoCo Do Simulation Test
TEST_CASE("MujocoEnv Simulation Step", "[MujocoEnv]") {
    MockMujocoEnv env("../datasets/mujoco_models/inverted_pendulum.xml");
    env.initialize_simulation();

    std::vector<double> control(env.m_->nu, 0.2);
    REQUIRE_NOTHROW(env.do_simulation(control, 5));

    for (int i = 0; i < env.m_->nu; i++) {
        CHECK(env.d_->ctrl[i] == Approx(0.2));
    }
}