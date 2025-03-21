#ifndef Mujoco_Ant_v4_h
#define Mujoco_Ant_v4_h

#include <MujocoEnv.h>
#include <misc.h>

class Mujoco_Ant_v4 : public MujocoEnv {
  public:
   // Parameters
   double control_cost_weight_ = 0.5;
   bool use_contact_forces_ = false;
   double contact_cost_weight_ = 5e-4;
   double healthy_reward_ = 1.0;
   bool terminate_when_unhealthy_ = true;
   std::vector<double> healthy_z_range_;
   std::vector<double> contact_force_range_;
   double reset_noise_scale_ = 0.1;
   bool exclude_current_positions_from_observation_ = true;

   Mujoco_Ant_v4(std::unordered_map<std::string, std::any>& params) {
      eval_type_ = "Mujoco";
      n_eval_train_ = std::any_cast<int>(params["mj_n_eval_train"]);
      n_eval_validation_ = std::any_cast<int>(params["mj_n_eval_validation"]);
      n_eval_test_ = std::any_cast<int>(params["mj_n_eval_test"]);
      max_step_ = std::any_cast<int>(params["mj_max_timestep"]);
      model_path_ =
          ExpandEnvVars(std::any_cast<string>(params["mj_model_path"]) + 
                        "ant.xml");
      healthy_z_range_ = {0.2, 1.0};
      contact_force_range_ = {-1.0, 1.0};
      initialize_simulation();

      obs_size_ = 27;
      if (!exclude_current_positions_from_observation_)
         obs_size_ += 2;
      if (use_contact_forces_)
         obs_size_ += 84;

      state_.resize(obs_size_);
   }

   ~Mujoco_Ant_v4() {
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

   std::vector<double> contact_forces() {
      std::vector<double> forces;
      std::copy_n(d_->cfrc_ext, m_->nbody * 6, back_inserter(forces));
      for (auto& f : forces) {
         f = std::max(contact_force_range_[0],
                      std::min(f, contact_force_range_[1]));
      }
      return forces;
   }

   double contact_cost() {
      auto forces = contact_forces();
      double cost = 0;
      for (auto& f : forces)
         cost += f * f;
      return contact_cost_weight_ * cost;
   }

   bool is_healthy() {
      for (int i = 0; i < m_->nq; i++)
         if (!std::isfinite(d_->qpos[i]))
            return false;
      for (int i = 0; i < m_->nv; i++)
         if (!std::isfinite(d_->qvel[i]))
            return false;
      return (d_->qpos[2] >= healthy_z_range_[0] &&
              d_->qpos[2] <= healthy_z_range_[1]);
   }

   bool terminal() {
      return step_ >= max_step_ || (terminate_when_unhealthy_ && !is_healthy());
   }

   Results sim_step(std::vector<double>& action) {
      auto x_pos_before = d_->qpos[0];
      do_simulation(action, frame_skip_);
      auto x_pos_after = d_->qpos[0];
      auto x_vel = (x_pos_after - x_pos_before) / m_->opt.timestep;
      auto forward_reward = x_vel;
      auto rewards = forward_reward + healthy_reward();
      auto ctrl_cost = control_cost(action);
      auto costs = ctrl_cost;
      if (use_contact_forces_) {
         costs += contact_cost();
      }
      auto reward = rewards - costs;
      get_obs(state_);
      step_++;
      return {reward, 0.0};  // TODO(skelly): maybe add gym 'info' to results
   }

   void get_obs(std::vector<double>& obs) {
      if (exclude_current_positions_from_observation_) {
         std::copy_n(d_->qpos + 2, m_->nq - 2, obs.begin());
         std::copy_n(d_->qvel, m_->nv, obs.begin() + (m_->nq - 2));
      } else {
         std::copy_n(d_->qpos, m_->nq, obs.begin());
         std::copy_n(d_->qvel, m_->nv, obs.begin() + m_->nq);
      }
   }

   void reset(mt19937& rng) {
      std::uniform_real_distribution<> dis_pos(-reset_noise_scale_,
                                               reset_noise_scale_);
      std::vector<double> qpos(m_->nq);
      for (size_t i = 0; i < qpos.size(); i++) {
         qpos[i] = init_qpos_[i] + dis_pos(rng);
      }
      std::normal_distribution<double> dis_vel(0.0, reset_noise_scale_);
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
