#ifndef Mujoco_Humanoid_Standup_v4_h
#define Mujoco_Humanoid_Standup_v4_h

#include <MujocoEnv.h>
#include <misc.h>

class Mujoco_Humanoid_Standup_v4 : public MujocoEnv {
  public:
   Mujoco_Humanoid_Standup_v4(
       std::unordered_map<std::string, std::any>& params) {
      eval_type_ = "Mujoco";
      n_eval_train_ = std::any_cast<int>(params["mj_n_eval_train"]);
      n_eval_validation_ = std::any_cast<int>(params["mj_n_eval_validation"]);
      n_eval_test_ = std::any_cast<int>(params["mj_n_eval_test"]);
      max_step_ = std::any_cast<int>(params["mj_max_timestep"]);
      model_path_ =
          ExpandEnvVars(std::any_cast<string>(params["mj_model_path"]));
      initialize_simulation();
      obs_size_ = 376;
      state_.resize(obs_size_);
   }

   ~Mujoco_Humanoid_Standup_v4() {
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

   bool terminal() { return step_ >= max_step_; }

   Results sim_step(std::vector<double>& action) {
      do_simulation(action, frame_skip_);
      auto pos_after = d_->qpos[2];
      auto uph_cost = (pos_after - 0) / m_->opt.timestep;

      double ctrl_square_sum = 0;
      double cfrc_ext_square_sum = 0;

      // Squaring and summing values
      for (auto i = 0; i < m_->nu; ++i) {
         ctrl_square_sum += d_->ctrl[i] * d_->ctrl[i];
      }

      for (auto i = 0; i < m_->nbody * 6; ++i) {
         cfrc_ext_square_sum += d_->cfrc_ext[i] * d_->cfrc_ext[i];
      }

      double quad_ctrl_cost = 0.1 * ctrl_square_sum;
      double quad_impact_cost = 0.5e-6 * cfrc_ext_square_sum;
      quad_impact_cost = std::min(quad_impact_cost, 10.0);

      double reward = uph_cost - quad_ctrl_cost - quad_impact_cost + 1.0;

      get_obs(state_);
      step_++;

      return {reward, 0.0};
   }

   void get_obs(std::vector<double>& obs) {
      size_t position_size = m_->nq - 2;  // excluding root position (qpos[2:])
      size_t velocity_size = m_->nv;
      size_t cinert_size = m_->nbody * 3;
      size_t cvel_size = m_->nbody * 6;
      size_t qfrc_size = m_->nu;
      size_t cfrc_size = m_->nbody * 6;

      std::copy_n(d_->qpos + 2, position_size, obs.begin());
      std::copy_n(d_->qvel, velocity_size, obs.begin() + position_size);
      std::copy_n(d_->cinert, cinert_size,
                  obs.begin() + position_size + velocity_size);
      std::copy_n(d_->cvel, cvel_size,
                  obs.begin() + position_size + velocity_size + cinert_size);
      std::copy_n(d_->qfrc_actuator, qfrc_size,
                  obs.begin() + position_size + velocity_size + cinert_size +
                      cvel_size);
      std::copy_n(d_->cfrc_ext, cfrc_size,
                  obs.begin() + position_size + velocity_size + cinert_size +
                      cvel_size + qfrc_size);
   }

   void reset(mt19937& rng) {
      double c = 0.01;

      std::uniform_real_distribution<> dis_pos(-c, c);
      std::vector<double> qpos(m_->nq);
      for (size_t i = 0; i < qpos.size(); i++) {
         qpos[i] = init_qpos_[i] + dis_pos(rng);
      }
      std::uniform_real_distribution<> dis_vel(-c, c);
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
