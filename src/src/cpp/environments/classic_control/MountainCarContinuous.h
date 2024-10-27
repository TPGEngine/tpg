#ifndef MountainCarContinuous_h
#define MountainCarContinuous_h

#include <ClassicControlEnv.h>
#include <math.h>
#include <stdlib.h>

#include <iostream>

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
    static constexpr int kPositionIndex = 0;
    static constexpr int kVelocityIndex = 1;

    double min_reward;

   public:
    bool discreteActions() const override { return false; }
    double maxActionContinuous() const override { return kMaxAction; }
    double minActionContinuous() const override { return kMinAction; }

    MountainCarContinuous() {
        n_eval_train_ = 20;
        n_eval_validation_ = 0;
        n_eval_test_ = 100;
        disReset = std::uniform_real_distribution<>(-0.6, -0.4);
        eval_type_ = "Control";
        max_step_ = 200;
        state_.reserve(kMountainCarContinuousStateSize);
        state_.resize(kMountainCarContinuousStateSize);
        state_po_.reserve(kMountainCarContinuousStateSize);
        state_po_.resize(kMountainCarContinuousStateSize);
    }

    ~MountainCarContinuous() {}

    void normalizeState(bool po) {
        if (po) {
            state_po_[kPositionIndex] = (state_po_[kPositionIndex] - kMinPosition) /
                                  (kMaxPosition - kMinPosition);
        }
    }

    void reset(mt19937 &rng) {
        state_[kPositionIndex] = state_po_[kPositionIndex] = disReset(rng);
        state_[kVelocityIndex] = 0;

        state_po_[kVelocityIndex] = disNoise(rng);

        state_[2] = disNoise(rng);
        state_[3] = disNoise(rng);

        reward = 0;

        step_ = 0;
        terminalState = false;
        normalizeState(true);
    }

    bool terminal() {
        if (step_ >= max_step_ || (state_[kPositionIndex] >= kGoalPosition &&
                                 state_[kVelocityIndex] >= kGoalVelocity))
            terminalState = true;
        return terminalState;
    }

    Results update(int actionD, double actionC, mt19937 &rng) {
        (void)actionD;
        double force = bound(actionC, kMinAction, kMaxAction);
        state_[kVelocityIndex] += force * kPower - kGravity * cos(3 * state_[kPositionIndex]);
        state_[kVelocityIndex] = bound(state_[kVelocityIndex], -kMaxSpeed, kMaxSpeed);
        state_[kPositionIndex] += state_[kVelocityIndex];
        state_[kPositionIndex] = bound(state_[kPositionIndex], kMinPosition, kMaxPosition);
        if (state_[kPositionIndex] == kMinPosition && state_[kVelocityIndex] < 0)
            state_[kVelocityIndex] = 0;

        state_po_[kPositionIndex] = state_[kPositionIndex];
        state_po_[kVelocityIndex] = disNoise(rng);

        state_[2] = disNoise(rng);
        state_[3] = disNoise(rng);

        step_++;

        // reward 2
        if (terminal() && step_ < max_step_)
            reward = 100;
        else
            reward = -(pow(force, 2) * 0.1);

        normalizeState(true);
        return {reward, 0.0};
    }

    // opengl
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
        vector<double> xs = linspace(kMinPosition, kMaxPosition, 100);
        for (size_t i = 1; i < xs.size() - 1; i++) {
            glVertex2d(x, sin(3 * xs[i]) * .45 + .55);
            if (state_[kPositionIndex] >= xs[i - 1] &&
                state_[kPositionIndex] <= xs[i + 1]) {
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
            double force = bound(actionC, kMinAction, kMaxAction);
            glLineWidth(2.0);
            drawTrace(0, "Action:", force, -1.0);
        }

        glColor3f(1.0, 1.0, 1.0);
        glLineWidth(1.0);
        drawEpisodeStepCounter(episode, step_, -1.9, -1.9);

        char c[80];
        if (step_ == 0)
            sprintf(c, "MountainCarContinuous Initial Conditions%s", ":");
        else if (terminal())
            sprintf(c, "MountainCarContinuous Terminal%s", ":");
        else
            sprintf(c, "Mountain Car%s", ":");
        drawStrokeText(c, -1.9, -1.7, 0);

        glFlush();
#endif
    }
};

#endif
