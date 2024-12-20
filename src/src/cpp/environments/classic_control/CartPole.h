#ifndef CartPole_h
#define CartPole_h


#include <math.h>
#include <stdlib.h>

#include <cmath>
#include <iostream>

#include <random>

#include "ClassicControlEnv.h"

#if !defined(CCANADA) && !defined(HPCC)
#include <GL/gl.h>
#include <GL/glut.h>
#endif


constexpr int kCartPoleStateSize = 4;

class CartPole : public ClassicControlEnv {
   protected:
    static constexpr double kGravity = 9.8;
    static constexpr double kMassCart = 1.0;
    static constexpr double kMassPole = 0.1;
    static constexpr double kTotalMass = (kMassPole + kMassCart);
    static constexpr double kLength = 0.5; /* Actually half the pole's length */
    static constexpr double kPoleMassLength = (kMassPole * kLength);
    static constexpr double kForceMag = 10.0;
    static constexpr double kTau = 0.02; /* Seconds between state updates */
    static constexpr double kFourThirds = 1.3333333333333;
    static constexpr double kSixDegrees = 0.1047198;
    static constexpr double kSevenDegrees = 0.1221730;
    static constexpr double kTenDegrees = 0.1745329;
    static constexpr double kTwelveDegrees = 0.2094384;
    static constexpr double kFifteenDegrees = 0.2617993;
    static constexpr double kTwelveDegreesSquared =
        kTwelveDegrees * kTwelveDegrees;

    static constexpr double kMinX = -1;
    static constexpr double kMaxX = 1;

    // State array indexing
    enum StateIndex { kX = 0, kTheta = 1, kXDot = 2, kThetaDot = 3 };

    int lastActionD = 0;

   public:
    CartPole() {
        n_eval_train_ = 20;
        n_eval_validation_ = 0;
        n_eval_test_ = 100;
        dis_reset = std::uniform_real_distribution<>(-0.05, 0.05);
        actionsDiscrete.push_back(-kForceMag);
        actionsDiscrete.push_back(0.0);
        actionsDiscrete.push_back(kForceMag);
        eval_type_ = "Control";
        max_step_ = 500;
        state_.reserve(kCartPoleStateSize);
        state_.resize(kCartPoleStateSize);
        state_po_.reserve(kCartPoleStateSize - 2);
        state_po_.resize(kCartPoleStateSize - 2);
    }

    ~CartPole() {
        state_.clear();
        state_po_.clear();
        actionsDiscrete.clear();
        action_trace.clear();
    }

    //! Returns the number of evaluations for a given phase (train, validation, or test)
    int GetNumEval(int phase) {
        if (phase == 0)
            return n_eval_train_;
        else if (phase == 1)
            return n_eval_validation_;
        else
            return n_eval_test_;
    }


    //! Normalizes the state values for position and angle
    void NormalizeState(bool po) {
        if (po) {
            state_po_[StateIndex::kX] /= kMaxX;
            state_po_[StateIndex::kTheta] /= kTwelveDegrees;
        }
    }
    
    //! Resets the CartPole environment to a initial state within specified ranges
    void Reset(std::mt19937 &rng) {
        state_po_[StateIndex::kX] = state_[StateIndex::kX] = dis_reset(rng);
        state_po_[StateIndex::kTheta] = state_[StateIndex::kTheta] =
            dis_reset(rng);
        state_[StateIndex::kXDot] = dis_reset(rng);
        state_[StateIndex::kThetaDot] = dis_reset(rng);
        reward = 0;
        step_ = 0;
        terminalState = false;
        NormalizeState(true);
    }

    //! Checks if the current state is terminal based on angle, position, or max steps
    bool Terminal() {
        if (step_ >= max_step_ ||
            std::abs(state_[StateIndex::kTheta]) > kTwelveDegrees ||
            std::abs(state_[StateIndex::kX]) > kMaxX)
            terminalState = true;
        return terminalState;
    }

    //! Updates the environment state based on the given action
    Results Update(int actionD, double actionC, std::mt19937 &rng) {
        double xacc, thetaacc, force, costheta, sintheta, temp;

        (void)actionC;
        (void)rng;

        // action 1 is ignored
        if (actionD == 0) {
            lastActionD = 0;
            force = actionsDiscrete[lastActionD];
        } else if (actionD == 2) {
            lastActionD = 2;
            force = actionsDiscrete[lastActionD];
        } else
            force = actionsDiscrete[lastActionD];

        // if (actionC < 0) force = actionsDiscrete[0];
        // else force = actionsDiscrete[2];

        costheta = std::cos(state_[StateIndex::kTheta]);
        sintheta = std::sin(state_[StateIndex::kTheta]);

        temp = (force + kPoleMassLength * state_[StateIndex::kThetaDot] *
                            state_[StateIndex::kThetaDot] * sintheta) /
               kTotalMass;

        thetaacc = (kGravity * sintheta - costheta * temp) /
                   (kLength * (kFourThirds -
                               kMassPole * costheta * costheta / kTotalMass));

        xacc = temp - kPoleMassLength * thetaacc * costheta / kTotalMass;

        /*** Update the four state variables, using Euler's method. ***/

        state_[StateIndex::kX] += kTau * state_[StateIndex::kXDot];
        state_po_[StateIndex::kX] = state_[StateIndex::kX];

        state_[StateIndex::kXDot] += kTau * xacc;

        state_[StateIndex::kTheta] += kTau * state_[StateIndex::kThetaDot];
        state_po_[StateIndex::kTheta] = state_[StateIndex::kTheta];

        state_[StateIndex::kThetaDot] += kTau * thetaacc;

        step_++;

        reward = 1.0;

        NormalizeState(true);
        return {reward, 0.0};
    }

    //! Displays the CartPole environment using OpenGL 
    void DisplayFunction(int episode, int actionD, double actionC) {
        (void)actionC;
        (void)actionD;
        (void)episode;
#if !defined(CCANADA) && !defined(HPCC)
        double r1 = 1.0;
        double x2, y2;

        glClear(GL_COLOR_BUFFER_BIT);

        glLineWidth(5.0);

        // cart
        glColor3f(0.0, 0.0, 1.0);
        glBegin(GL_TRIANGLES);
        glVertex2f(state_[StateIndex::kX] - 0.15, 0.075);
        glVertex2f(state_[StateIndex::kX] - 0.15, -0.075);
        glVertex2f(state_[StateIndex::kX] + 0.15, 0.075);
        glVertex2f(state_[StateIndex::kX] + 0.15, 0.075);
        glVertex2f(state_[StateIndex::kX] - 0.15, -0.075);
        glVertex2f(state_[StateIndex::kX] + 0.15, -0.075);
        glEnd();

        // pole
        x2 = state_[StateIndex::kX] +
             r1 * std::cos(M_PI / 2 - state_[StateIndex::kTheta]);
        y2 = r1 * std::sin(M_PI / 2 - state_[StateIndex::kTheta]);
        glColor3f(1.0, 1.0, 1.0);
        glBegin(GL_LINES);
        glVertex2d(state_[StateIndex::kX], 0.0);
        glVertex2d(x2, y2);

        // x bounds surface
        glVertex2d(kMinX, 0.0);
        glVertex2d(kMaxX, 0.0);

        glEnd();

        // discrete action arrows
        if (step_ > 0) {
            // action 1 is ignored
            double force = 0;
            if (actionD == 0)
                force = actionsDiscrete[0];
            else if (actionD == 2)
                force = actionsDiscrete[2];

            int dir = 1;
            if (actionD == 0)
                dir = -1;
            else if (actionD == 2)
                dir = 1;
            glBegin(GL_POLYGON);
            glVertex3f(dir * 0.12, -0.2, 0);
            glVertex3f(dir * 0.25, -0.25, 0);
            glVertex3f(dir * 0.12, -0.3, 0);
            glEnd();
            glLineWidth(2.0);
            DrawTrace(0, "Action:", force / kForceMag, -1.0);
        }

        glLineWidth(1.0);
        DrawEpisodeStepCounter(episode, step_, -1.9, -1.9);

        glColor3f(1.0, 1.0, 1.0);
        char c[80];
        if (step_ == 0)
            std::sprintf(c, "CartPole Initial Conditions%s", ":");
        else if (Terminal())
            std::sprintf(c, "CartPole Terminal%s", ":");
        else
            std::sprintf(c, "CartPole%s", ":");

        DrawStrokeText(c, -1.9, -1.7, 0);

        glFlush();
#endif
    }
};

#endif  