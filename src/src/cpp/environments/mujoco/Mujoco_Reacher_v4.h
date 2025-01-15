#ifndef Mujoco_Reacher_v4_h
#define Mujoco_Reacher_v4_h

#include <MujocoEnv.h>
#include <misc.h>

class Mujoco_Reacher_v4 : public MujocoEnv {
  private:
   int id_fingertip_;
   int id_target_;

  public:
   // Parameters
   double reward_distance_weight = 1.0;
   double reward_control_weight = 0.1;
   Mujoco_Reacher_v4(std::unordered_map<std::string, std::any>& params) {
      eval_type_ = "Mujoco";
      n_eval_train_ = std::any_cast<int>(params["mj_n_eval_train"]);
      n_eval_validation_ = std::any_cast<int>(params["mj_n_eval_validation"]);
      n_eval_test_ = std::any_cast<int>(params["mj_n_eval_test"]);
      max_step_ = std::any_cast<int>(params["mj_max_timestep"]);
      model_path_ =
          ExpandEnvVars(std::any_cast<string>(params["mj_model_path"]));

      initialize_simulation();

      id_fingertip_ = mj_name2id(m_, mjOBJ_XBODY, "fingertip");
      id_target_ = mj_name2id(m_, mjOBJ_XBODY, "target");

      obs_size_ = 10;
      state_.resize(obs_size_);
   }

   ~Mujoco_Reacher_v4() {
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

   double control_cost(std::vector<double>& action) {
      double cost = 0;
      for (auto& a : action)
         cost += a * a;
      return reward_control_weight * cost;
   }

   std::vector<double> get_dist() {
      return {d_->xpos[id_fingertip_ * 3 + 0] - d_->xpos[id_target_ * 3 + 0],
              d_->xpos[id_fingertip_ * 3 + 1] - d_->xpos[id_target_ * 3 + 1]};
   }

   bool terminal() { return step_ >= max_step_; }

   Results sim_step(std::vector<double>& action) {
      auto dist_diff = get_dist();
      double reward_dist =
          -reward_distance_weight *
          std::sqrt(std::pow(dist_diff[0], 2) + std::pow(dist_diff[1], 2));

      double reward_ctrl = -control_cost(action);

      reward = reward_dist + reward_ctrl;
      do_simulation(action, frame_skip_);
      get_obs(state_);
      step_++;
      return {reward, 0.0};  // TODO(skelly): maybe add gym 'info' to results
   }

   void get_obs(std::vector<double>& obs) {
      // get 10 obs : theta = qpos[0:2]
      std::vector<double> theta(2);
      std::copy_n(d_->qpos, 2, theta.begin());
      std::vector<double> cos_theta(2);
      std::vector<double> sin_theta(2);
      std::transform(theta.begin(), theta.end(), cos_theta.begin(),
                     [](double x) { return cos(x); });
      std::transform(theta.begin(), theta.end(), sin_theta.begin(),
                     [](double x) { return sin(x); });
      // cos(theta)
      std::copy_n(cos_theta.begin(), 2, obs.begin());
      // sin(theta)
      std::copy_n(sin_theta.begin(), 2, obs.begin() + 2);
      // qpos[2:]
      std::copy_n(d_->qpos + 2, m_->nq - 2, obs.begin() + 4);
      // qvel[0:2]
      std::copy_n(d_->qvel, 2, obs.begin() + 4 + m_->nq - 2);
      // xpos[0:2]
      auto dist_diff = get_dist();
      std::copy_n(dist_diff.begin(), 2, obs.begin() + 6 + m_->nq - 2);
   }

   void reset(mt19937& rng) {

      std::uniform_real_distribution<> dis_pos(-0.1, 0.1);
      std::vector<double> qpos(m_->nq);
      for (size_t i = 0; i < qpos.size(); i++) {
         qpos[i] = init_qpos_[i] + dis_pos(rng);
      }

      std::vector<double> goal(2);
      std::normal_distribution<double> dis_goal(-0.2, 0.2);
      while (true) {

         goal[0] = dis_goal(rng);
         goal[1] = dis_goal(rng);

         if (goal[0] * goal[0] + goal[1] * goal[1] < 0.04)
            break;
      }

      std::copy_n(goal.begin(), 2, qpos.end() - 2);

      std::normal_distribution<double> dis_vel(-0.005, 0.005);
      std::vector<double> qvel(m_->nv);
      for (size_t i = 0; i < qvel.size(); i++) {
         qvel[i] = init_qvel_[i] + dis_vel(rng);
      }
      std::fill_n(qvel.begin() + 2, 2, 0.0);
      mj_resetData(m_, d_);
      set_state(qpos, qvel);
      step_ = 0;
   }
};

#endif
