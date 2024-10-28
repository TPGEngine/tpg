#ifndef MountainCar_h
#define MountainCar_h

#include <ClassicControlEnv.h>
#include <math.h>
#include <stdlib.h>

#include <iostream>

#if !defined(CCANADA) && !defined(HPCC)
#include <GL/gl.h>
#include <GL/glut.h>
#endif

// Number of state variables in the mountain car environment
constexpr int kMountainCarStateSize = 4;

class MountainCar : public ClassicControlEnv {
   protected:
    // Environment constants
    static constexpr double kMinPosition = -1.2;
    static constexpr double kMaxPosition = 0.6;
    static constexpr double kMaxSpeed = 0.07;
    static constexpr double kGoalPosition = 0.5;
    static constexpr double kGoalVelocity = 0.0;
    static constexpr double kForce = 0.001;
    static constexpr double kGravity = 0.0025;

    // State array indices
    enum StateIndex {
        kPosition = 0,
        kVelocity = 1,
        kNoise1 = 2,
        kNoise2 = 3
    };

   public:
    MountainCar() {
        n_eval_train_ = 20;
        n_eval_validation_ = 0;
        n_eval_test_ = 100;
        // TODO: refactor with ClassicControlEnv
        disReset = std::uniform_real_distribution<>(-0.6, -0.4);
        eval_type_ = "Control";
        max_step_ = 200;
        state_.reserve(kMountainCarStateSize);
        state_.resize(kMountainCarStateSize);
        state_po_.reserve(kMountainCarStateSize);
        state_po_.resize(kMountainCarStateSize);
    }

    ~MountainCar() {}

    void NormalizeState(bool partially_observable) {
        if (partially_observable) {
            state_po_[StateIndex::kPosition] = (state_po_[StateIndex::kPosition] - kMinPosition) /
                                      (kMaxPosition - kMinPosition);
        }
    }

    // TODO: refactor with TaskEnv
    void reset(std::mt19937& rng) override {
        state_[StateIndex::kPosition] = state_po_[StateIndex::kPosition] = disReset(rng);
        state_[StateIndex::kVelocity] = 0;
        state_po_[StateIndex::kVelocity] = disNoise(rng);

        state_[StateIndex::kNoise1] = disNoise(rng);
        state_[StateIndex::kNoise2] = disNoise(rng);

        reward = 0;
        step_ = 0;
        terminalState = false;
        NormalizeState(true);
    }

    // TODO: refactor with TaskEnv
    bool terminal() override {
        if (step_ >= max_step_ || 
            (state_[StateIndex::kPosition] >= kGoalPosition)) {
            terminalState = true;
        }
        return terminalState;
    }

    // TODO: refactor with TaskEnv
    Results update(int action_discrete, double action_continuous, 
                  std::mt19937& rng) override {
        (void)action_continuous;  // Unused parameter

        state_[StateIndex::kVelocity] += 
            (action_discrete - 1) * kForce + 
            std::cos(3 * state_[StateIndex::kPosition]) * -kGravity;
            
        state_[StateIndex::kVelocity] = 
            bound(state_[StateIndex::kVelocity], -kMaxSpeed, kMaxSpeed);
        state_[StateIndex::kPosition] += state_[StateIndex::kVelocity];
        state_[StateIndex::kPosition] = 
            bound(state_[StateIndex::kPosition], kMinPosition, kMaxPosition);

        if (state_[StateIndex::kPosition] == kMinPosition && 
            state_[StateIndex::kVelocity] < 0) {
            state_[StateIndex::kVelocity] = 0;
        }

        state_po_[StateIndex::kPosition] = state_[StateIndex::kPosition];
        state_po_[StateIndex::kVelocity] = disNoise(rng);

        state_[StateIndex::kNoise1] = disNoise(rng);
        state_[StateIndex::kNoise2] = disNoise(rng);

        step_++;

        reward = -1.0;

        NormalizeState(true);
        Results result = {reward, 0.0};
        return result;
    }

    // opengl
    void display_function(int episode, int actionD, double actionC) {
        (void)episode;
        (void)actionC;
        (void)actionD;
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
            if (state_[StateIndex::kPosition] >= xs[i - 1] &&
                state_[StateIndex::kPosition] <= xs[i + 1]) {
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
            // action arrows
            int dir = 1;
            if (actionD == 0)
                dir = -1;
            else if (actionD == 2)
                dir = 1;
            glColor3f(1.0, 1.0, 1.0);
            glBegin(GL_POLYGON);
            glVertex3f(dir * 0.12, -0.2, 0);
            glVertex3f(dir * 0.25, -0.25, 0);
            glVertex3f(dir * 0.12, -0.3, 0);
            glEnd();
            glLineWidth(2.0);
            drawTrace(0, "Action:", actionD - 1, -1.0);
        }
        drawEpisodeStepCounter(episode, step_, -1.9, 1.3);
        glFlush();
#endif
    }
};
#endif
