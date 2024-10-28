#ifndef Acrobot_h
#define Acrobot_h

#include <math.h>
#include <stdlib.h>

#include <iostream>

#include "ClassicControlEnv.h"

#if !defined(CCANADA) && !defined(HPCC)
#include <GL/gl.h>
#include <GL/glut.h>
#endif

#define STATE_SIZE 4

class Acrobot : public ClassicControlEnv {
   protected:
    const double maxTheta1 = M_PI;
    const double maxTheta2 = M_PI;
    const double maxTheta1Dot = 4 * M_PI;
    const double maxTheta2Dot = 9 * M_PI;
    const double m1 = 1.0;
    const double m2 = 1.0;
    const double l1 = 1.0;
    const double l2 = 1.0;
    const double lc1 = 0.5;
    const double lc2 = 0.5;
    const double I1 = 1.0;
    const double I2 = 1.0;
    const double g = 9.8;
    const double dt = 0.05;
    const double AcrobotGoalPosition = 1.0;

    // state array indexing
    const int _theta1 = 0;
    const int _theta2 = 1;
    const int _theta1Dot = 2;
    const int _theta2Dot = 3;

    const int _theta1_po = 0;
    const int _theta2_po = 1;

   public:
    Acrobot() {
        n_eval_train_ = 20;
        n_eval_validation_ = 0;
        n_eval_test_ = 100;
        dis_reset = std::uniform_real_distribution<>(-0.1, 0.1);
        actionsDiscrete.push_back(-1.0);
        actionsDiscrete.push_back(0.0);
        actionsDiscrete.push_back(1.0);
        eval_type_ = "Control";
        max_step_ = 200;
        state_.reserve(STATE_SIZE);
        state_.resize(STATE_SIZE);
        state_po_.reserve(STATE_SIZE);
        state_po_.resize(STATE_SIZE);
    }

    ~Acrobot() {}

    bool discreteActions() const { return false; }

    double minActionContinuous() const { return -1.0; }

    double maxActionContinuous() const { return 1.0; }

    //! Normalizes the state values by dividing them by their respective maximum values
    void normalizeState(bool po) {
        if (po) {
            state_po_[_theta1] /= maxTheta1;
            state_po_[_theta2] /= maxTheta2;
        }
    }

    //! Resets the Acrobot environment to a initial state within specified ranges - uniform_real_distribution<>(-0.1, 0.1);
    void reset(std::mt19937& rng) {
        state_po_[_theta1] = state_[_theta1] = dis_reset(rng);

        state_po_[_theta2] = state_[_theta2] = dis_reset(rng);

        state_[_theta1Dot] = dis_reset(rng);

        state_[_theta2Dot] = dis_reset(rng);

        reward = 0;

        step_ = 0;
        terminalState = false;

        normalizeState(true);
    }

    //! Updates Acrobot state based on the given action and returns the reward 
    Results update(int actionD, double actionC, std::mt19937& rng) {
        (void)actionD;
        (void)rng;

        // double torque = actionsDiscrete[actionD];
        double torque = Bound(actionC, -1.0, 1.0);
        double d1;
        double d2;
        double phi_2;
        double phi_1;

        double theta2_ddot;
        double theta1_ddot;

        int count = 0;
        while (!terminal() && count < 4) {
            count++;

            d1 = m1 * pow(lc1, 2) +
                 m2 * (pow(l1, 2) + pow(lc2, 2) +
                       2 * l1 * lc2 * cos(state_[_theta2])) +
                 I1 + I2;
            d2 = m2 * (pow(lc2, 2) + l1 * lc2 * cos(state_[_theta2])) + I2;

            phi_2 = m2 * lc2 * g *
                    cos(state_[_theta1] + state_[_theta2] - M_PI / 2.0);
            phi_1 =
                -(m2 * l1 * lc2 * pow(state_[_theta2Dot], 2) *
                      sin(state_[_theta2]) -
                  2 * m2 * l1 * lc2 * state_[_theta1Dot] * state_[_theta2Dot] *
                      sin(state_[_theta2])) +
                (m1 * lc1 + m2 * l1) * g * cos(state_[_theta1] - M_PI / 2.0) +
                phi_2;

            theta2_ddot = (torque + (d2 / d1) * phi_1 -
                           m2 * l1 * lc2 * pow(state_[_theta1Dot], 2) *
                               sin(state_[_theta2]) -
                           phi_2) /
                          (m2 * pow(lc2, 2) + I2 - pow(d2, 2) / d1);

            theta1_ddot = -(d2 * theta2_ddot + phi_1) / d1;

            state_[_theta1Dot] += theta1_ddot * dt;
            state_[_theta2Dot] += theta2_ddot * dt;

            state_[_theta1] += state_[_theta1Dot] * dt;
            state_[_theta2] += state_[_theta2Dot] * dt;
            state_[_theta1] = wrap(state_[_theta1], -maxTheta1, maxTheta1);
            state_[_theta2] = wrap(state_[_theta2], -maxTheta2, maxTheta2);
            state_[_theta1Dot] =
                Bound(state_[_theta1Dot], -maxTheta1Dot, maxTheta1Dot);
            state_[_theta2Dot] =
                Bound(state_[_theta2Dot], -maxTheta2Dot, maxTheta2Dot);
        }

        state_po_[_theta1] = state_[_theta1];
        state_po_[_theta2] = state_[_theta2];

        step_++;

        reward = -1.0;

        normalizeState(true);
        return {reward, 0.0};
    }

    //! Provide boolean result to check if the current state is terminal based on step count or position
    bool terminal() {
        if (step_ >= max_step_ ||
            (-cos(state_[_theta1]) - cos(state_[_theta2] + state_[_theta1]) >
             AcrobotGoalPosition))
            terminalState = true;
        return terminalState;
    }

    //! Wraps a value x within the range [m, M] by adding or subtracting the difference 
    double wrap(double x, double m, double M) {
        double diff = M - m;
        while (x > M)
            x = x - diff;
        while (x < m)
            x = x + diff;
        return x;
    }


    //! Renders the current state of the Acrobot environment using OpenGL
    void display_function(int episode, int actionD, double actionC) {
        (void)episode;
        (void)actionD;
        (void)actionC;
#if !defined(CCANADA) && !defined(HPCC)
        double r1 = 1.0;
        double r2 = 1.0;
        double x2, y2, x3, y3;
        glClear(GL_COLOR_BUFFER_BIT);

        glLineWidth(5.0);

        x2 = r1 * cos(M_PI / 2 - state_[_theta1]);
        y2 = r1 * sin(M_PI / 2 - state_[_theta1]);
        glColor3f(1.0, 1.0, 1.0);
        glBegin(GL_LINES);
        glVertex2d(0.0, 0.0);
        glVertex2d(-x2, -y2);

        x3 = x2 + r2 * cos(M_PI / 2 - (state_[_theta1] + state_[_theta2]));
        y3 = y2 + r2 * sin(M_PI / 2 - (state_[_theta1] + state_[_theta2]));
        glColor3f(1.0, 1.0, 1.0);
        glBegin(GL_LINES);
        glVertex2d(-x2, -y2);
        glVertex2d(-x3, -y3);

        // surface
        glVertex2d(-1.5, 0.0);
        glVertex2d(1.5, 0.0);

        // goal
        glColor3f(0.0, 1.0, 0.0);
        glVertex2d(-1.5, r1);
        glVertex2d(1.5, r1);

        glEnd();

        if (step_ > 0) {
            glColor3f(1.0, 1.0, 1.0);
            double torque = Bound(actionC, -1.0, 1.0);
            glLineWidth(2.0);
            DrawTrace(0, "Action:", torque / 1.0, 1.2);
        }

        glColor3f(1.0, 1.0, 1.0);
        glLineWidth(1.0);
        DrawEpisodeStepCounter(episode, step_, -1.9, -1.9);

        char c[80];
        if (step_ == 0)
            sprintf(c, "Acrobot Initial Conditions%s", ":");
        else if (terminal())
            sprintf(c, "Acrobot Terminal%s", ":");
        else
            sprintf(c, "Acrobot%s", ":");
        DrawStrokeText(c, -1.9, -1.7, 0);

        glFlush();
#endif
    }
};

#endif
