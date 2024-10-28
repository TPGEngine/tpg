#ifndef Mujoco_Inverted_Pendulum_v4_h
#define Mujoco_Inverted_Pendulum_v4_h

#include <MujocoEnv.h>
#include <misc.h>

class Mujoco_Inverted_Pendulum_v4 : public MujocoEnv {
   public:
    // Parameters
    // double control_cost_weight_ = 0.5;
    // bool use_contact_forces_ = false;
    // double contact_cost_weight_ = 5e-4;
    // double healthy_reward_ = 1.0;
    // bool terminate_when_unhealthy_ = true;
    // std::vector<double> healthy_z_range_;
    // std::vector<double> contact_force_range_;
    // double reset_noise_scale_ = 0.1;
    // bool exclude_current_positions_from_observation_ = false;

    Mujoco_Inverted_Pendulum_v4(std::unordered_map<std::string, std::any>& params) {
        eval_type_ = "Mujoco";
        n_eval_train_ = std::any_cast<int>(params["mj_n_eval_train"]);
        n_eval_validation_ = 0;
        n_eval_test_ = n_eval_train_ =
            std::any_cast<int>(params["mj_n_eval_test"]);
        max_step_ = std::any_cast<int>(params["mj_max_timestep"]);
        model_path_ =
            ExpandEnvVars(std::any_cast<string>(params["mj_model_path"]));
        initialize_simulation();
        obs_size_ = 4; 
        state_.resize(obs_size_);
    }

    ~Mujoco_Inverted_Pendulum_v4() {
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

    bool terminal() {
        bool has_non_finite = std::any_of(state_.begin(), state_.end(), [](double x) { return !std::isfinite(x); });
        bool second_element_large = std::abs(state_[1]) > 0.2;
        return has_non_finite || second_element_large;
    }

    Results sim_step(std::vector<double>& action) {
        auto reward = 1.0;
        do_simulation(action, frame_skip_);
        get_obs(state_);
        step_++;
        return {reward, 0.0};  // TODO(skelly): maybe add gym 'info' to results
    }

    void get_obs(std::vector<double>& obs) {
        std::copy_n(d_->qpos, m_->nq, obs.begin());
        std::copy_n(d_->qvel, m_->nv, obs.begin() + m_->nq);
    }

    void reset(mt19937& rng) {
        std::uniform_real_distribution<> dis_pos(-0.01,
                                                 0.01);
        std::vector<double> qpos(m_->nq);
        for (size_t i = 0; i < qpos.size(); i++) {
            qpos[i] = init_qpos_[i] + dis_pos(rng);
        }
        std::normal_distribution<double> dis_vel(-0.01, 0.01);
        std::vector<double> qvel(m_->nv);
        for (size_t i = 0; i < qvel.size(); i++) {
            qvel[i] = init_qvel_[i] + dis_vel(rng);
        }
        mj_resetData(m_, d_);
        set_state(qpos, qvel);
        step_ = 0;
    }
};

#endif