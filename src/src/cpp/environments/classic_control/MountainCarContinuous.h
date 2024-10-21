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

#define STATE_SIZE 4

class MountainCarContinuous : public ClassicControlEnv {
   protected:
    const double min_action = -1.0;
    const double max_action = 1.0;
    const double min_position = -1.2;
    const double max_position = 0.6;
    const double max_speed = 0.07;
    const double goal_position = 0.45;
    const double goal_velocity = 0.0;
    const double power = 0.0015;
    const double gravity = 0.0025;

    // state array indexing
    const int _position = 0;
    const int _velocity = 1;

    double min_reward;

   public:
    bool discreteActions() const { return false; }
    double maxActionContinuous() const { return max_action; }
    double minActionContinuous() const { return min_action; }

    MountainCarContinuous() {
        n_eval_train_ = 20;
        n_eval_validation_ = 0;
        n_eval_test_ = 100;
        disReset = uniform_real_distribution<>(-0.6, -0.4);
        eval_type_ = "Control";
        max_step_ = 200;
        state_.reserve(STATE_SIZE);
        state_.resize(STATE_SIZE);
        state_po_.reserve(STATE_SIZE);
        state_po_.resize(STATE_SIZE);
    }

    ~MountainCarContinuous() {}

    void normalizeState(bool po) {
        if (po) {
            state_po_[_position] = (state_po_[_position] - min_position) /
                                   (max_position - min_position);
        }
    }

    void reset(mt19937& rng) {
        state_[_position] = state_po_[_position] = disReset(rng);
        state_[_velocity] = 0;

        state_po_[_velocity] = disNoise(rng);

        state_[2] = disNoise(rng);
        state_[3] = disNoise(rng);

        reward = 0;

        step_ = 0;
        terminalState = false;
        normalizeState(true);
    }

    bool terminal() {
        if (step_ >= max_step_ || (state_[_position] >= goal_position &&
                                   state_[_velocity] >= goal_velocity))
            terminalState = true;
        return terminalState;
    }

    Results update(int actionD, double actionC, mt19937& rng) {
        (void)actionD;
        double force = bound(actionC, min_action, max_action);
        state_[_velocity] +=
            force * power - gravity * cos(3 * state_[_position]);
        state_[_velocity] = bound(state_[_velocity], -max_speed, max_speed);
        state_[_position] += state_[_velocity];
        state_[_position] =
            bound(state_[_position], min_position, max_position);
        if (state_[_position] == min_position && state_[_velocity] < 0)
            state_[_velocity] = 0;

        state_po_[_position] = state_[_position];
        state_po_[_velocity] = disNoise(rng);

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
        vector<double> xs = linspace(min_position, max_position, 100);
        for (size_t i = 1; i < xs.size() - 1; i++) {
            glVertex2d(x, sin(3 * xs[i]) * .45 + .55);
            if (state_[_position] >= xs[i - 1] &&
                state_[_position] <= xs[i + 1]) {
                carX = x;
                carXS = xs[i];
            }
            if (goal_position >= xs[i - 1] && goal_position <= xs[i + 1]) {
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
            double force = bound(actionC, min_action, max_action);
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
