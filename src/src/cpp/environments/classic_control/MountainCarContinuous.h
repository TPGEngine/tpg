#ifndef MountainCarContinuous_h
#define MountainCarContinuous_h

#include <math.h>
#include <stdlib.h>

#include <iostream>

#include "ClassicControlEnv.h"

#if !defined(CCANADA) && !defined(HPCC)
#include <GL/gl.h>
#include <GL/glut.h>
#endif

// Mountain car environment with continuous actions
constexpr int kMountainCarContinuousStateSize = 4;

class MountainCarContinuous : public ClassicControlEnv {
   protected:
    // Environment constants
    static constexpr double kMinAction = -1.0;
    static constexpr double kMaxAction = 1.0;
    static constexpr double kMinPosition = -1.2;
    static constexpr double kMaxPosition = 0.6;
    static constexpr double kMaxSpeed = 0.07;
    static constexpr double kGoalPosition = 0.45;
    static constexpr double kGoalVelocity = 0.0;
    static constexpr double kPower = 0.0015;
    static constexpr double kGravity = 0.0025;

    // State array indices
    enum StateIndex {
        kPosition = 0,
        kVelocity = 1,
        kNoise1 = 2,
        kNoise2 = 3
    };

    double min_reward;

   public:
    bool DiscreteActions() const override { return false; }
    double MaxActionContinuous() const override { return kMaxAction; }
    double MinActionContinuous() const override { return kMinAction; }

    MountainCarContinuous() {
        n_eval_train_ = 20;
        n_eval_validation_ = 0;
        n_eval_test_ = 100;
        dis_reset = std::uniform_real_distribution<>(-0.6, -0.4);
        eval_type_ = "Control";
        max_step_ = 200;
        state_.reserve(kMountainCarContinuousStateSize);
        state_.resize(kMountainCarContinuousStateSize);
        state_po_.reserve(kMountainCarContinuousStateSize);
        state_po_.resize(kMountainCarContinuousStateSize);
    }

    ~MountainCarContinuous() {}

    //! Normalizes the state values for partially observable environments
    void NormalizeState(bool po) {
        if (po) {
            state_po_[kPosition] = (state_po_[kPosition] - kMinPosition) /
                                  (kMaxPosition - kMinPosition);
        }
    }
    //! Resets the environment to its initial state
    void reset(std::mt19937 &rng) {
        state_[kPosition] = state_po_[kPosition] = dis_reset(rng);
        state_[kVelocity] = 0;

        state_po_[kVelocity] = dis_noise(rng);

        state_[2] = dis_noise(rng);
        state_[3] = dis_noise(rng);

        reward = 0;

        step_ = 0;
        terminalState = false;
        NormalizeState(true);
    }

    //! Checks if the current state is terminal based on steps/position/velocity
    bool Terminal() {
        if (step_ >= max_step_ || (state_[kPosition] >= kGoalPosition &&
                                 state_[kVelocity] >= kGoalVelocity))
            terminalState = true;
        return terminalState;
    }

    //! Updates the environment based on the given action
    Results Update(int actionD, double actionC, std::mt19937& rng) {
        (void)actionD;
        double force = Bound(actionC, kMinAction, kMaxAction);
        state_[kVelocity] += force * kPower - kGravity * cos(3 * state_[kPosition]);
        state_[kVelocity] = Bound(state_[kVelocity], -kMaxSpeed, kMaxSpeed);
        state_[kPosition] += state_[kVelocity];
        state_[kPosition] = Bound(state_[kPosition], kMinPosition, kMaxPosition);
        if (state_[kPosition] == kMinPosition && state_[kVelocity] < 0)
            state_[kVelocity] = 0;

        state_po_[kPosition] = state_[kPosition];
        state_po_[kVelocity] = dis_noise(rng);

        state_[2] = dis_noise(rng);
        state_[3] = dis_noise(rng);

        step_++;

        // reward 2
        if (Terminal() && step_ < max_step_)
            reward = 100;
        else
            reward = -(pow(force, 2) * 0.1);

        NormalizeState(true);
        return {reward, 0.0};
    }

    //! Displays the current state of the environment using OpenGL
    void display_function(int episode, int actionD, double actionC) {
        (void)episode;
        (void)actionD;
        (void)actionC;
#if !defined(CCANADA) && !defined(HPCC)
        glClear(GL_COLOR_BUFFER_BIT);

        glLineWidth(5.0);

        // track
        glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);
        glPointSize(1);
        glColor3f(1.0, 1.0, 1.0);
        glBegin(GL_LINES);
        double carX = 0;
        double carXS = 0;
        double goalX = 0;
        double goalXS = 0;
        double x = -2.0;
        std::vector<double> xs = Linspace(kMinPosition, kMaxPosition, 100);
        for (size_t i = 1; i < xs.size() - 1; i++) {
            glVertex2d(x, sin(3 * xs[i]) * .45 + .55);
            if (state_[kPosition] >= xs[i - 1] &&
                state_[kPosition] <= xs[i + 1]) {
                carX = x;
                carXS = xs[i];
            }
            if (kGoalPosition >= xs[i - 1] && kGoalPosition <= xs[i + 1]) {
                goalX = x;
                goalXS = xs[i];
            }
            x = x + 0.04;
        }
        glEnd();

        // car
        glBegin(GL_LINES);
        glColor3f(1.0, 0.0, 0.0);
        glVertex2d(carX, (sin(3 * carXS) * .45 + .55) + 0.1);
        glVertex2d(carX, (sin(3 * carXS) * .45 + .55) - 0.1);
        glEnd();

        // goal
        glBegin(GL_LINES);
        glColor3f(0.0, 1.0, 0.0);
        glVertex2d(goalX, (sin(3 * goalXS) * .45 + .55) + 0.1);
        glVertex2d(goalX, (sin(3 * goalXS) * .45 + .55) - 0.1);
        glEnd();

        if (step_ > 0) {
            double force = Bound(actionC, kMinAction, kMaxAction);
            glLineWidth(2.0);
            DrawTrace(0, "Action:", force, -1.0);
        }

        glColor3f(1.0, 1.0, 1.0);
        glLineWidth(1.0);
        DrawEpisodeStepCounter(episode, step_, -1.9, -1.9);

        char c[80];
        if (step_ == 0)
            sprintf(c, "MountainCarContinuous Initial Conditions%s", ":");
        else if (terminal())
            sprintf(c, "MountainCarContinuous Terminal%s", ":");
        else
            sprintf(c, "Mountain Car%s", ":");
        DrawStrokeText(c, -1.9, -1.7, 0);

        glFlush();
#endif
    }
};

#endif
