#ifndef Acrobot_h
#define Acrobot_h

#include <math.h>
#include <stdlib.h>

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <random>

#include "ClassicControlEnv.h"

#include "ClassicControlEnv.h"

#if !defined(CCANADA) && !defined(HPCC)
#include <GL/gl.h>
#include <GL/glut.h>
#endif


constexpr int kAcrobotStateSize = 4;


class Acrobot : public ClassicControlEnv {
   protected:
    static constexpr double kMaxTheta1 = M_PI;
    static constexpr double kMaxTheta2 = M_PI;
    static constexpr double kMaxTheta1Dot = 4 * M_PI;
    static constexpr double kMaxTheta2Dot = 9 * M_PI;
    static constexpr double kM1 = 1.0;
    static constexpr double kM2 = 1.0;
    static constexpr double kL1 = 1.0;
    static constexpr double kLC1 = 0.5;
    static constexpr double kLC2 = 0.5;
    static constexpr double kI1 = 1.0;
    static constexpr double kI2 = 1.0;
    static constexpr double kG = 9.8;
    static constexpr double kDt = 0.05;
    static constexpr double kAcrobotGoalPosition = 1.0;

    // State array indexing
    enum StateIndex {
        kTheta1 = 0,
        kTheta2 = 1,
        kTheta1Dot = 2,
        kTheta2Dot = 3
    };
    enum StatePoIndex { kTheta1Po = 0, kTheta2Po = 1 };

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
        state_.reserve(kAcrobotStateSize);
        state_.resize(kAcrobotStateSize);
        state_po_.reserve(kAcrobotStateSize);
        state_po_.resize(kAcrobotStateSize);
    }

    ~Acrobot() {}

    // TODO: Change function name once TaskEnv follows Google's C++ Styling
    bool discreteActions() const { return false; }

    // TODO: Change function name once TaskEnv follows Google's C++ Styling
    double minActionContinuous() const { return -1.0; }

    // TODO: Change function name once TaskEnv follows Google's C++ Styling
    double maxActionContinuous() const { return 1.0; }

    //! Normalizes the state values by dividing them by their respective maximum values
    void normalizeState(bool po) {

    void NormalizeState(bool po) {
        if (po) {
            state_po_[StateIndex::kTheta1] /= kMaxTheta1;
            state_po_[StateIndex::kTheta2] /= kMaxTheta2;
        }
    }

    //! Resets the Acrobot environment to a initial state within specified ranges - uniform_real_distribution<>(-0.1, 0.1);
    void reset(std::mt19937 &rng) {
        state_po_[StateIndex::kTheta1] = state_[StateIndex::kTheta1] =
            dis_reset(rng);

        state_po_[StateIndex::kTheta2] = state_[StateIndex::kTheta2] =
            dis_reset(rng);

        state_[StateIndex::kTheta1Dot] = dis_reset(rng);

        state_[StateIndex::kTheta2Dot] = dis_reset(rng);


        reward = 0;

        step_ = 0;
        terminalState = false;

        NormalizeState(true);
    }


    // TODO: Change function name once TaskEnv follows Google's C++ Styling
    //! Updates Acrobot state based on the given action and returns the reward 
    Results update(int actionD, double actionC, std::mt19937 &rng) {

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

            d1 =
                kM1 * std::pow(kLC1, 2) +
                kM2 * (std::pow(kL1, 2) + std::pow(kLC2, 2) +
                       2 * kL1 * kLC2 * std::cos(state_[StateIndex::kTheta2])) +
                kI1 + kI2;
            d2 = kM2 * (std::pow(kLC2, 2) +
                        kL1 * kLC2 * std::cos(state_[StateIndex::kTheta2])) +
                 kI2;

            phi_2 = kM2 * kLC2 * kG *
                    std::cos(state_[StateIndex::kTheta1] +
                             state_[StateIndex::kTheta2] - M_PI / 2.0);
            phi_1 = -(kM2 * kL1 * kLC2 *
                          std::pow(state_[StateIndex::kTheta2Dot], 2) *
                          std::sin(state_[StateIndex::kTheta2]) -
                      2 * kM2 * kL1 * kLC2 * state_[StateIndex::kTheta1Dot] *
                          state_[StateIndex::kTheta2Dot] *
                          std::sin(state_[StateIndex::kTheta2])) +
                    (kM1 * kLC1 + kM2 * kL1) * kG *
                        std::cos(state_[StateIndex::kTheta1] - M_PI / 2.0) +
                    phi_2;

            theta2_ddot =
                (torque + (d2 / d1) * phi_1 -
                 kM2 * kL1 * kLC2 *
                     std::pow(state_[StateIndex::kTheta1Dot], 2) *
                     std::sin(state_[StateIndex::kTheta2]) -
                 phi_2) /
                (kM2 * std::pow(kLC2, 2) + kI2 - std::pow(d2, 2) / d1);

            theta1_ddot = -(d2 * theta2_ddot + phi_1) / d1;

            state_[StateIndex::kTheta1Dot] += theta1_ddot * kDt;
            state_[StateIndex::kTheta2Dot] += theta2_ddot * kDt;

            state_[StateIndex::kTheta1] += state_[StateIndex::kTheta1Dot] * kDt;
            state_[StateIndex::kTheta2] += state_[StateIndex::kTheta2Dot] * kDt;
            state_[StateIndex::kTheta1] =
                Wrap(state_[StateIndex::kTheta1], -kMaxTheta1, kMaxTheta1);
            state_[StateIndex::kTheta2] =
                Wrap(state_[StateIndex::kTheta2], -kMaxTheta2, kMaxTheta2);
            state_[StateIndex::kTheta1Dot] = bound(
                state_[StateIndex::kTheta1Dot], -kMaxTheta1Dot, kMaxTheta1Dot);
            state_[StateIndex::kTheta2Dot] = bound(
                state_[StateIndex::kTheta2Dot], -kMaxTheta2Dot, kMaxTheta2Dot);

        }

        state_po_[StateIndex::kTheta1] = state_[StateIndex::kTheta1];
        state_po_[StateIndex::kTheta2] = state_[StateIndex::kTheta2];

        step_++;

        reward = -1.0;

        NormalizeState(true);
        return {reward, 0.0};
    }


    //! Provide boolean result to check if the current state is terminal based on step count or position
    // TODO: Change function name once TaskEnv follows Google's C++ Styling

    bool terminal() {
        if (step_ >= max_step_ || (-std::cos(state_[StateIndex::kTheta1]) -
                                       std::cos(state_[StateIndex::kTheta2] +
                                                state_[StateIndex::kTheta1]) >
                                   kAcrobotGoalPosition))
            terminalState = true;
        return terminalState;
    }


    //! Wraps a value x within the range [m, M] by adding or subtracting the difference 
    double Wrap(double x, double m, double M) {
        double diff = M - m;
        while (x > M)
            x = x - diff;
        while (x < m)
            x = x + diff;
        return x;
    }

    //! Renders the current state of the Acrobot environment using OpenGL
    // TODO: Change function name once TaskEnv follows Google's C++ Styling
    // OpenGL Display
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

        x2 = r1 * std::cos(M_PI / 2 - state_[StateIndex::kTheta1]);
        y2 = r1 * std::sin(M_PI / 2 - state_[StateIndex::kTheta1]);
        glColor3f(1.0, 1.0, 1.0);
        glBegin(GL_LINES);
        glVertex2d(0.0, 0.0);
        glVertex2d(-x2, -y2);

        x3 = x2 + r2 * std::cos(M_PI / 2 - (state_[StateIndex::kTheta1] +
                                            state_[StateIndex::kTheta2]));
        y3 = y2 + r2 * std::sin(M_PI / 2 - (state_[StateIndex::kTheta1] +
                                            state_[StateIndex::kTheta2]));
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
            std::sprintf(c, "Acrobot Initial Conditions%s", ":");
        else if (terminal())
            std::sprintf(c, "Acrobot Terminal%s", ":");
        else

            std::sprintf(c, "Acrobot%s", ":");
        DrawStrokeText(c, -1.9, -1.7, 0);
        glFlush();
#endif
    }
};

#endif  // ACROBOT_H
