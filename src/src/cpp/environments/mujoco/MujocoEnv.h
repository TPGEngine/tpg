#ifndef MujocoEnv_h
#define MujocoEnv_h

#include <TaskEnv.h>
//#include <GLFW/glfw3.h>
#include <mujoco/mujoco.h>
#include <algorithm>
#include <iostream>

class MujocoEnv : public TaskEnv {
   public:
    // MuJoCo data structures
    mjModel* m_ = NULL;  // MuJoCo model
    mjData* d_ = NULL;   // MuJoCo data
    mjvCamera cam_;      // abstract camera
    mjvOption opt_;      // visualization options
    mjvScene scn_;       // abstract scene
    mjrContext con_;     // custom GPU context

    std::vector<double> init_qpos_;  // Initial positions
    std::vector<double> init_qvel_;  // Initial velocities

    std::string model_path_;  // Absolute path to model xml file
    int frame_skip_ = 1;  // Number of frames per simlation step
    int obs_size_;  // Number of variables in observation vector

    MujocoEnv() {}
    ~MujocoEnv() {}
    virtual void reset(std::mt19937& rng) = 0;
    virtual bool terminal() = 0;
    virtual Results sim_step(std::vector<double>& action) = 0;

    void initialize_simulation() {
        // Load and compile model
        char error[1000] = "Could not load binary model";
        m_ = mj_loadXML(model_path_.c_str(), 0, error, 1000);
        // Make data
        d_ = mj_makeData(m_);

        // Proceed with the rest of the initialization
        std::copy_n(d_->qpos, m_->nq, back_inserter(init_qpos_));
        std::copy_n(d_->qvel, m_->nv, back_inserter(init_qvel_));
    }

    void set_state(std::vector<double>& qpos, std::vector<double>& qvel) {
        // Set the joints position qpos and velocity qvel of the model.
        // Note: `qpos` and `qvel` is not the full physics state for all mujoco
        // models/environments
        // https://mujoco.readthedocs.io/en/stable/APIreference/APItypes.html#mjtstate

        for (int i = 0; i < m_->nq; i++) d_->qpos[i] = qpos[i];
        for (int i = 0; i < m_->nv; i++) d_->qvel[i] = qvel[i];
        mj_forward(m_, d_);
    }

    void do_simulation(std::vector<double>& ctrl, int n_frames) {
        for (int i = 0; i < m_->nu && i < static_cast<int>(ctrl.size()); i++) {
            d_->ctrl[i] = ctrl[i];
        }
        for (int i = 0; i < n_frames; i++) {
            mj_step(m_, d_);
        }
        // As of MuJoCo 2.0, force - related quantities like cacc are not
        // computed unless there's a force sensor in the model. See https:
        // // github.com/openai/gym/issues/1541
        mj_rnePostConstraint(m_, d_);
    }

    int GetObsSize() { return obs_size_; }
};

#endif
