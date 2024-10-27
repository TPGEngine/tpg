#ifndef ACROBOT_H_
#define ACROBOT_H_

#include <math.h>

#include <cstdlib>
#include <iostream>
#include <random>

#include "ClassicControlEnv.h"

#if !defined(CCANADA) && !defined(HPCC)
#include <GL/gl.h>
#include <GL/glut.h>
#endif

#define STATE_SIZE 4

class Acrobot : public ClassicControlEnv {
   protected:
    const double kMaxTheta1 = M_PI;
    const double kMaxTheta2 = M_PI;
    const double kMaxTheta1Dot = 4 * M_PI;
    const double kMaxTheta2Dot = 9 * M_PI;
    const double kM1 = 1.0;
    const double kM2 = 1.0;
    const double kL1 = 1.0;
    const double kLC1 = 0.5;
    const double kLC2 = 0.5;
    const double kI1 = 1.0;
    const double kI2 = 1.0;
    const double kG = 9.8;
    const double kDt = 0.05;
    const double kAcrobotGoalPosition = 1.0;

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
        disReset = std::uniform_real_distribution<>(-0.1, 0.1);
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

    // TODO: Change function name once TaskEnv follows Google's C++ Styling
    bool discreteActions() const { return false; }

    // TODO: Change function name once TaskEnv follows Google's C++ Styling
    double minActionContinuous() const { return -1.0; }

    // TODO: Change function name once TaskEnv follows Google's C++ Styling
    double maxActionContinuous() const { return 1.0; }

    void NormalizeState(bool po) {
        if (po) {
            state_po_[StateIndex::kTheta1] /= kMaxTheta1;
            state_po_[StateIndex::kTheta2] /= kMaxTheta2;
        }
    }

    // TODO: Change function name once TaskEnv follows Google's C++ Styling
    void reset(std::mt19937 &rng) {
        state_po_[StateIndex::kTheta1] = state_[StateIndex::kTheta1] =
            disReset(rng);

        state_po_[StateIndex::kTheta2] = state_[StateIndex::kTheta2] =
            disReset(rng);

        state_[StateIndex::kTheta1Dot] = disReset(rng);

        state_[StateIndex::kTheta2Dot] = disReset(rng);

        reward = 0;

        step_ = 0;
        terminalState = false;

        NormalizeState(true);
    }

    // TODO: Change function name once TaskEnv follows Google's C++ Styling
    Results update(int actionD, double actionC, std::mt19937 &rng) {
        (void)actionD;
        (void)rng;

        // double torque = actionsDiscrete[actionD];
        double torque = bound(actionC, -1.0, 1.0);
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

    // TODO: Change function name once TaskEnv follows Google's C++ Styling
    bool terminal() {
        if (step_ >= max_step_ || (-std::cos(state_[StateIndex::kTheta1]) -
                                       std::cos(state_[StateIndex::kTheta2] +
                                                state_[StateIndex::kTheta1]) >
                                   kAcrobotGoalPosition))
            terminalState = true;
        return terminalState;
    }

    double Wrap(double x, double m, double M) {
        double diff = M - m;
        while (x > M) x = x - diff;
        while (x < m) x = x + diff;
        return x;
    }

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
            double torque = bound(actionC, -1.0, 1.0);
            glLineWidth(2.0);
            drawTrace(0, "Action:", torque / 1.0, 1.2);
        }

        glColor3f(1.0, 1.0, 1.0);
        glLineWidth(1.0);
        drawEpisodeStepCounter(episode, step_, -1.9, -1.9);

        char c[80];
        if (step_ == 0)
            std::sprintf(c, "Acrobot Initial Conditions%s", ":");
        else if (terminal())
            std::sprintf(c, "Acrobot Terminal%s", ":");
        else
            std::sprintf(c, "Acrobot%s", ":");
        drawStrokeText(c, -1.9, -1.7, 0);

        glFlush();
#endif
    }
};

#endif  // ACROBOT_H
