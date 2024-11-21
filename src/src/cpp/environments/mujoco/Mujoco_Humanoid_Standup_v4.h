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

      double quad_ctrl_cost =
          0.1 * std::accumulate(d_->ctrl.begin(), d_->ctrl.end(), 0.0,
                                [](double acc, double value) {
                                   return acc + value * value;
                                });
      double quad_impact_cost =
          0.5e-6 * std::accumulate(d_->cfrc_ext.begin(), d_->cfrc_ext.end(),
                                   0.0, [](double acc, double value) {
                                      return acc + value * value;
                                   });
      quad_impact_cost = std::min(quad_impact_cost, 10.0);

      double reward = uph_cost - quad_ctrl_cost - quad_impact_cost + 1.0;

      get_obs(state_);
      step_++;

      return {reward, 0.0};
   }

   void get_obs(std::vector<double>& obs) {
      std::vector<double> obs;

      obs.insert(obs.end(), d_->qpos.begin() + 2, d_->qpos.end());
      obs.insert(obs.end(), d_->qvel.begin(), d_->qvel.end());
      obs.insert(obs.end(), d_->cinert.begin(), d_->cinert.end());
      obs.insert(obs.end(), d_->cvel.begin(), d_->cvel.end());
      obs.insert(obs.end(), d_->qfrc_actuator.begin(), d_->qfrc_actuator.end());
      obs.insert(obs.end(), d_->cfrc_ext.begin(), d_->cfrc_ext.end());

      return obs;
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