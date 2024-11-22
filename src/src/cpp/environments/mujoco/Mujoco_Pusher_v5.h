#ifndef Mujoco_Pusher_v5_h
#define Mujoco_Pusher_v5_h

#include <MujocoEnv.h>
#include <misc.h>

class Mujoco_Pusher_v5 : public MujocoEnv {

   std::vector<double> cylinder_pos_;
   std::vector<double> goal_pos_;

   Mujoco_Pusher_v5(std::unordered_map<std::string, std::any>& params) {
      eval_type_ = "Mujoco";
      n_eval_train_ = std::any_cast<int>(params["mj_n_eval_train"]);
      n_eval_validation_ = std::any_cast<int>(params["mj_n_eval_validation"]);
      n_eval_test_ = std::any_cast<int>(params["mj_n_eval_test"]);
      max_step_ = std::any_cast<int>(params["mj_max_timestep"]);
      model_path_ =
          ExpandEnvVars(std::any_cast<string>(params["mj_model_path"]));
      initialize_simulation();
      obs_size_ = 23;
      state_.resize(obs_size_);
   }

   ~Mujoco_Pusher_v5() {
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
      auto vec_1 = d_->qpos[0];
      auto vec_2 = d_->qpos[0];

      double reward_near = -vec_1.norm() * reward_near_weight_;
      double reward_dist = -vec_2.norm() * reward_dist_weight_;
      double reward_ctrl = -std::inner_product(action.begin(), action.end(), action.begin(), 0.0) * reward_control_weight_;

      double reward = reward_dist + reward_ctrl + reward_near;

      // Update the state and increment step
      step_++;
      return {reward, 0.0};  
   }

   void get_obs(std::vector<double>& obs) {
      std::copy_n(d_->qpos, m_->nq, obs.begin());
      std::copy_n(d_->qvel, m_->nv, obs.begin() + m_->nq);
   }

   void reset(mt19937& rng) {
      std::vector<double> qpos(m_->nq);
      goal_pos_ = {0.45, -0.05, -0.323};
      while (true) {
         cylinder_pos_ = {
            std::uniform_real_distribution<double>(-0.3, 0)(rng),
            std::uniform_real_distribution<double>(-0.2, 0.2)(rng)
         };
         if ((cylinder_pos_ - goal_pos_).norm() > 0.17) {
            break;
         }
      }

      qpos[m_->nq - 4] = cylinder_pos_[0];
      qpos[m_->nq - 3] = cylinder_pos_[1];
      qpos[m_->nq - 2] = goal_pos_[0];
      qpos[m_->nq - 1] = goal_pos_[1];

      std::vector<double> qvel(m_->nv);
      for (size_t i = 0; i < qvel.size(); i++) {
         qvel[i] = init_qvel_[i] + std::uniform_real_distribution<double>(-0.005, 0.005)(rng);
      }
      std::fill(qvel.end() - 4, qvel.end(), 0.0);
      set_state(qpos, qvel);
   }
};

#endif