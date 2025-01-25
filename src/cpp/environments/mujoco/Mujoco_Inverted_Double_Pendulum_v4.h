#ifndef Mujoco_Inverted_Double_Pendulum_v4_h
#define Mujoco_Inverted_Double_Pendulum_v4_h

#include <MujocoEnv.h>
#include <misc.h>
#include <cmath>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <any>

class Mujoco_Inverted_Double_Pendulum_v4 : public MujocoEnv {
public:
    Mujoco_Inverted_Double_Pendulum_v4(std::unordered_map<std::string, std::any>& params) {
        eval_type_ = "Mujoco";
        n_eval_train_ = std::any_cast<int>(params["mj_n_eval_train"]);
        n_eval_validation_ = std::any_cast<int>(params["mj_n_eval_validation"]);
        n_eval_test_ = std::any_cast<int>(params["mj_n_eval_test"]);
        max_step_ = std::any_cast<int>(params["mj_max_timestep"]);
        model_path_ = ExpandEnvVars(std::any_cast<std::string>(params["mj_model_path"]));

        initialize_simulation();

        obs_size_ = 11;
        state_.resize(obs_size_);
    }

    ~Mujoco_Inverted_Double_Pendulum_v4() {
        // Free visualization storage
        mjv_freeScene(&scn_);
        mjr_freeContext(&con_);

        // Free MuJoCo model and data
        mj_deleteData(d_);
        mj_deleteModel(m_);

        // Terminate GLFW (crashes with Linux NVidia drivers)
    #if defined(__APPLE__) || defined(_WIN32)
        glfwTerminate();
    #endif
    }

    Results sim_step(std::vector<double>& action) {
        do_simulation(action, frame_skip_);
        get_obs(state_);

        // Get x and y from d_->site_xpos[0], site index 0
        double x = d_->site_xpos[0]; // x-coordinate
        double y = d_->site_xpos[2]; // z-coordinate (vertical axis)

        double dist_penalty = 0.01 * x * x + (y - 2.0) * (y - 2.0);

        double v1 = d_->qvel[1];
        double v2 = d_->qvel[2];

        double vel_penalty = 1e-3 * v1 * v1 + 5e-3 * v2 * v2;

        double alive_bonus = 10.0;

        reward = alive_bonus - dist_penalty - vel_penalty;

        step_++;

        // Update terminal state
        terminalState = (y <= 1.0) || (step_ >= max_step_);

        return {reward, 0.0}; // TODO: add 'info' if needed
    }

    void get_obs(std::vector<double>& obs) {
        // obs[0]: cart x position
        obs[0] = d_->qpos[0];

        // obs[1], obs[2]: sin(qpos[1]), sin(qpos[2])
        obs[1] = sin(d_->qpos[1]);
        obs[2] = sin(d_->qpos[2]);

        // obs[3], obs[4]: cos(qpos[1]), cos(qpos[2])
        obs[3] = cos(d_->qpos[1]);
        obs[4] = cos(d_->qpos[2]);

        // obs[5], obs[6], obs[7]: qvel[0], qvel[1], qvel[2], clipped to [-10,10]
        for (int i = 0; i < 3; i++) {
            obs[5 + i] = std::max(std::min(d_->qvel[i], 10.0), -10.0);
        }

        // obs[8], obs[9], obs[10]: qfrc_constraint[0], [1], [2], clipped to [-10,10]
        for (int i = 0; i < 3; i++) {
            obs[8 + i] = std::max(std::min(d_->qfrc_constraint[i], 10.0), -10.0);
        }
    }

    void reset(std::mt19937& rng) {
        std::uniform_real_distribution<double> dis_pos(-0.1, 0.1);
        std::vector<double> qpos(m_->nq);
        for (size_t i = 0; i < qpos.size(); i++) {
            qpos[i] = init_qpos_[i] + dis_pos(rng);
        }

        std::normal_distribution<double> dis_vel(0.0, 0.1);
        std::vector<double> qvel(m_->nv);
        for (size_t i = 0; i < qvel.size(); i++) {
            qvel[i] = init_qvel_[i] + dis_vel(rng);
        }

        mj_resetData(m_, d_);
        set_state(qpos, qvel);

        step_ = 0;
        terminalState = false;
    }

    bool terminal() {
        // Check for non-finite elements
        for (int i = 0; i < m_->nq; i++)
            if (!std::isfinite(d_->qpos[i]))
                return true;
        for (int i = 0; i < m_->nv; i++)
            if (!std::isfinite(d_->qvel[i]))
                return true;

        // Get y coordinate (vertical axis) of site[0], which is z-axis
        double y = d_->site_xpos[2]; // site_xpos[0] corresponds to site 0, index 2 is z-coordinate

        return step_ >= max_step_ || (y <= 1.0);
    }

    double reward; // reward for current step
};

#endif
