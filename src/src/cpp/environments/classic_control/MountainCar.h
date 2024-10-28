#ifndef MountainCar_h
#define MountainCar_h

#include <math.h>
#include <stdlib.h>

#include <iostream>

#include "ClassicControlEnv.h"

#if !defined(CCANADA) && !defined(HPCC)
#include <GL/gl.h>
#include <GL/glut.h>
#endif

#define STATE_SIZE 4

class MountainCar : public ClassicControlEnv {
   protected:
    const double min_position = -1.2;
    const double max_position = 0.6;
    const double max_speed = 0.07;
    const double goal_position = 0.5;
    const double goal_velocity = 0.0;
    const double force = 0.001;
    const double gravity = 0.0025;

    // state array indexing
    const int _position = 0;
    const int _velocity = 1;

   public:
    MountainCar() {
        n_eval_train_ = 20;
        n_eval_validation_ = 0;
        n_eval_test_ = 100;
        dis_reset = std::uniform_real_distribution<>(-0.6, -0.4);
        eval_type_ = "Control";
        max_step_ = 200;
        state_.reserve(STATE_SIZE);
        state_.resize(STATE_SIZE);
        state_po_.reserve(STATE_SIZE);
        state_po_.resize(STATE_SIZE);
    }

    ~MountainCar() {}

    //! Normalizes the state values for partially observable environments
    void normalizeState(bool po) {
        if (po)
            state_po_[_position] = (state_po_[_position] - min_position) /
                                   (max_position - min_position);
    }

    //! Resets the environment to its initial state within specified ranges
    void reset(mt19937& rng) {
        state_[_position] = state_po_[_position] = dis_reset(rng);
        state_[_velocity] = 0;

        state_po_[_velocity] = dis_noise(rng);

        state_[2] = dis_noise(rng);
        state_[3] = dis_noise(rng);

        reward = 0;

        step_ = 0;
        terminalState = false;
        normalizeState(true);
    }

    //! Checks if the current state is terminal based on steps or position
    bool terminal() {
        if (step_ >= max_step_ ||
            (state_[_position] >=
             goal_position))  // && state_[_velocity] >= goal_velocity))
            terminalState = true;
        return terminalState;
    }

    //! Updates the environment based on the given action
    Results update(int actionD, double actionC, mt19937& rng) {
        (void)actionC;

        state_[_velocity] +=
            (actionD - 1) * force + cos(3 * state_[_position]) * -gravity;
        state_[_velocity] = bound(state_[_velocity], -max_speed, max_speed);
        state_[_position] += state_[_velocity];
        state_[_position] =
            bound(state_[_position], min_position, max_position);
        if (state_[_position] == min_position && state_[_velocity] < 0)
            state_[_velocity] = 0;

        state_po_[_position] = state_[_position];
        state_po_[_velocity] = dis_noise(rng);

        state_[2] = dis_noise(rng);
        state_[3] = dis_noise(rng);

        step_++;

        reward = -1.0;

        normalizeState(true);
        return {reward, 0.0};
    }

    //! Displays the current state of the environment using OpenGL
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
        vector<double> xs = Linspace(min_position, max_position, 100);
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
            DrawTrace(0, "Action:", actionD - 1, -1.0);
        }
        DrawEpisodeStepCounter(episode, step_, -1.9, 1.3);
        glFlush();
#endif
    }
};
#endif
