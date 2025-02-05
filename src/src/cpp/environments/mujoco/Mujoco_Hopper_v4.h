#ifndef Mujoco_Hopper_v4_h
#define Mujoco_Hopper_v4_h

#include <MujocoEnv.h>
#include <misc.h>
#include <iostream>

class Mujoco_Hopper_v4 : public MujocoEnv {
  public:
   // Parameters
   double forward_reward_weight = 1.0;
   double control_cost_weight_ = 1e-3;
   double healthy_reward_ = 1.0;
   bool terminate_when_unhealthy_ = true;
   std::vector<double> healthy_state_range_;
   std::vector<double> healthy_z_range_;
   std::vector<double> healthy_angle_range_;
   double reset_noise_scale_ = 5e-3;
   bool exclude_current_positions_from_observation_ = true;

   Mujoco_Hopper_v4(std::unordered_map<std::string, std::any>& params) {
      eval_type_ = "Mujoco";
      n_eval_train_ = std::any_cast<int>(params["mj_n_eval_train"]);
      n_eval_validation_ = std::any_cast<int>(params["mj_n_eval_validation"]);
      n_eval_test_ = std::any_cast<int>(params["mj_n_eval_test"]);
      max_step_ = std::any_cast<int>(params["mj_max_timestep"]);
      control_cost_weight_ =
          std::any_cast<double>(params["mj_reward_control_weight"]);
      model_path_ =
          ExpandEnvVars(std::any_cast<string>(params["mj_model_path"])+
                        "hopper.xml");
      healthy_state_range_ = {-100.0, 100.0};
      healthy_z_range_ = {0.7, float(INFINITY)};
      healthy_angle_range_ = {-0.2, 0.2};
      initialize_simulation();

      obs_size_ = 12;
      if (exclude_current_positions_from_observation_)
         obs_size_ = 11;

      state_.resize(obs_size_);
   }

   ~Mujoco_Hopper_v4() {
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

   double healthy_reward() {
      return static_cast<double>(is_healthy() || terminate_when_unhealthy_) *
             healthy_reward_;
   }

   double control_cost(std::vector<double>& action) {
      double cost = 0;
      for (auto& a : action)
         cost += a * a;
      return control_cost_weight_ * cost;
   }

   bool is_healthy() {
      double z = d_->qpos[1];
      double angle = d_->qpos[2];

      bool healthy_state = true;
      for (double s : state_) {
         if (!(healthy_state_range_[0] < s && s < healthy_state_range_[1])) {
            healthy_state = false;
            break;
         }
      }

      bool healthy_z = healthy_z_range_[0] < z && z < healthy_z_range_[1];
      bool healthy_angle =
          healthy_angle_range_[0] < angle && angle < healthy_angle_range_[1];
      return healthy_state && healthy_z && healthy_angle;
   }

   bool terminal() {
      return step_ >= max_step_ || (terminate_when_unhealthy_ && !is_healthy());
   }

   Results sim_step(std::vector<double>& action) {
      auto x_pos_before = d_->qpos[0];
      do_simulation(action, frame_skip_);
      auto x_pos_after = d_->qpos[0];
      auto x_vel = (x_pos_after - x_pos_before) / m_->opt.timestep;
      auto ctrl_cost = control_cost(action);

      auto forward_reward = forward_reward_weight * x_vel;
      auto rewards = forward_reward + healthy_reward();

      auto costs = ctrl_cost;
      auto reward = rewards - costs;

      get_obs(state_);
      step_++;
      return {reward, 0.0};
   }

   void get_obs(std::vector<double>& obs) {
      auto position_size =
          exclude_current_positions_from_observation_ ? m_->nq - 1 : m_->nq;

      if (exclude_current_positions_from_observation_) {
         std::copy_n(d_->qpos + 1, position_size, state_.begin());
      } else {
         std::copy_n(d_->qpos, position_size, state_.begin());
      }
      std::copy_n(d_->qvel, m_->nv, state_.begin() + position_size);

      for (int i = 0; i < m_->nv; ++i) {
         state_[position_size + i] =
             std::clamp(state_[position_size + i], -10.0, 10.0);
      }
   }

   void reset(mt19937& rng) {
      std::uniform_real_distribution<> dis_pos(-reset_noise_scale_,
                                               reset_noise_scale_);
      std::vector<double> qpos(m_->nq);
      for (size_t i = 0; i < qpos.size(); i++) {
         qpos[i] = init_qpos_[i] + dis_pos(rng);
      }
      std::uniform_real_distribution<> dis_vel(0.0, reset_noise_scale_);
      std::vector<double> qvel(m_->nv);
      for (size_t i = 0; i < qvel.size(); i++) {
         qvel[i] = init_qvel_[i] + dis_vel(rng);
      }
      mj_resetData(m_, d_);
      set_state(qpos, qvel);
      step_ = 0;
   };
};

#endif