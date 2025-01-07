#ifndef CartCentering_h
#define CartCentering_h

#include <math.h>

#include <misc.h>
#include <stdlib.h>

#include <cmath>
#include <iostream>

#include <random>

#include "ClassicControlEnv.h"

#if !defined(CCANADA) && !defined(HPCC)
#include <GL/gl.h>
#include <GL/glut.h>
#endif


constexpr int kCartCenteringStateSize = 4;

class CartCentering : public ClassicControlEnv {
   protected:
    /*** Parameters for simulation ***/
    static constexpr double kMassCart = 2.0;
    static constexpr double kForceMag = 1.0;
    static constexpr double kTau = 0.02;  // seconds between state updates

    // These may depend on each other and dt
    static constexpr double kMaxX = 1.5;
    static constexpr double kMaxV = 6;

    static constexpr double kMinVarIni = -0.75;
    static constexpr double kMaxVarIni = 0.75;
    static constexpr double kNearOrigin = 0.01;

    // State array indexing
    enum StateIndex { kX = 0, kV = 1 };


    uniform_real_distribution<> dis_reset;
    int last_action_d = -kForceMag;

   public:
    CartCentering() {
        n_eval_train_ = 20;
        n_eval_validation_ = 0;
        n_eval_test_ = 100;
        dis_reset = std::uniform_real_distribution<>(kMinVarIni, kMaxVarIni);
        actionsDiscrete.push_back(-kForceMag);
        actionsDiscrete.push_back(0.0);
        actionsDiscrete.push_back(kForceMag);
        eval_type_ = "Control";
        max_step_ = 500;
        state_.reserve(kCartCenteringStateSize);
        state_.resize(kCartCenteringStateSize);
        state_po_.reserve(kCartCenteringStateSize);
        state_po_.resize(kCartCenteringStateSize);
    }

    ~CartCentering() {}

    void NormalizeState(bool po) {
        if (po) {
            state_po_[StateIndex::kX] /= kMaxX;
            state_po_[StateIndex::kV] /= M_PI;
        }
    }


   //! Resets the CartCentering environment to a initial state within specified ranges
    void Reset(std::mt19937 &rng) {
        step_ = 0;

        do {
            state_po_[StateIndex::kX] = state_[StateIndex::kX] = dis_reset(rng);
            state_[StateIndex::kV] = dis_reset(rng);
            terminalState = false;
        } while (Terminal());

        state_po_[StateIndex::kV] = dis_noise(rng);


        state_[2] = dis_noise(rng);
        state_[3] = dis_noise(rng);

        reward = 0;

        NormalizeState(true);
    }


    /****************************************************************************/
    //! Checks if the current state is terminal based on step count, position, and velocity

    bool Terminal() {
        terminalState =
            step_ >= max_step_ ||
                    (std::abs(state_[StateIndex::kX]) <= kNearOrigin &&
                     std::abs(state_[StateIndex::kV]) <= kNearOrigin) ||
                    std::abs(state_[StateIndex::kX]) > kMaxX
                ? true
                : false;
        return terminalState;
    }

    /****************************************************************************/
    //! Updates the environment state based on the given action and returns the reward
    Results Update(int actionD, double actionC, std::mt19937 &rng) {
        (void)actionC;
        double force;

        // action 1 is ignored
        if (actionD == 0) {
            last_action_d = 0;
            force = actionsDiscrete[last_action_d];
        } else if (actionD == 2) {
            last_action_d = 2;
            force = actionsDiscrete[last_action_d];
        } else
            force = actionsDiscrete[last_action_d];

        double acc_t = force / kMassCart;

        state_[StateIndex::kX] += kTau * state_[StateIndex::kV];
        state_po_[StateIndex::kX] = state_[StateIndex::kX];


        state_[StateIndex::kV] += kTau * acc_t;
        state_[StateIndex::kV] = bound(state_[StateIndex::kV], -kMaxV, kMaxV);
        state_po_[StateIndex::kV] = dis_noise(rng);


        state_[2] = dis_noise(rng);
        state_[3] = dis_noise(rng);

        step_++;

        if (Terminal())
            reward = -((((std::abs(state_[StateIndex::kX]) / kMaxX) +
                         (std::abs(state_[StateIndex::kV]) / kMaxV) / 2)) +
                       (((double)step_ / max_step_) * 0.1));
        else
            reward = 0;

        NormalizeState(true);

        return {reward, 0.0};
    }

    /****************************************************************************/
    //! Renders the current state of the CartCentering environment using OpenGL
    // OpenGL Display
    void DisplayFunction(int episode, int actionD, double actionC) {
        (void)episode;
        (void)actionD;
        (void)actionC;
#if !defined(CCANADA) && !defined(HPCC)
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
        glColor3f(1.0, 1.0, 1.0);
        glBegin(GL_LINES);
        glVertex2d(state_[StateIndex::kX], -0.075);
        glVertex2d(state_[StateIndex::kX], 0.075);

        // x bounds surface
        glVertex2d(-kMaxX, 0.0);
        glVertex2d(kMaxX, 0.0);

        // ticks (green centre) TMP
        for (int i = 1; kMaxX - (i * 0.1) > -kMaxX; i++) {
            glVertex2d(kMaxX - (i * 0.1), 0.0);
            if (isEqual(kMaxX - (i * 0.1), 0)) {
                glColor3f(0.0, 1.0, 0.0);
                glVertex2d(1.5 - (i * 0.1),
                           isEqual(1.5 - (i * 0.1), 0) ? -0.1 : -0.025);
                glColor3f(1.0, 1.0, 1.0);
            }
        }
        glEnd();

        if (step_ > 0) {
            // discrete action arrows, action 1 is ignored
            double force = 0;
            if (actionD == 0)
                force = actionsDiscrete[0];
            else if (actionD == 2)
                force = actionsDiscrete[2];
            else
                force = actionsDiscrete[last_action_d];

            int dir = 1;
            if (force < 0)
                dir = -1;
            else if (force > 0)
                dir = 1;
            glBegin(GL_POLYGON);
            glVertex3f(dir * 0.12, -0.2, 0);
            glVertex3f(dir * 0.25, -0.25, 0);
            glVertex3f(dir * 0.12, -0.3, 0);
            glEnd();


            if (std::abs(state_[StateIndex::kX]) <= kNearOrigin &&
                std::abs(state_[StateIndex::kV]) <= kNearOrigin) {
                glColor3f(0.0, 1.0, 0.0);
                glBegin(GL_LINE_LOOP);
                for (int i = 0; i <= 300; i++) {
                    double angle = 2 * M_PI * i / 300;
                    double x = cos(angle);
                    double y = sin(angle);
                    glVertex2d(x, y);
                }
            }
            glEnd();
            glLineWidth(2.0);
            DrawTrace(0, "Action:", force / kForceMag, -1.0);
        }

        glColor3f(1.0, 1.0, 1.0);
        glLineWidth(1.0);
        DrawEpisodeStepCounter(episode, step_, -1.9, -1.9);

        char c[80];
        if (step_ == 0)
            std::sprintf(c, "CartCentering Initial Conditions%s", ":");
        else if (Terminal())
            std::sprintf(c, "CartCentering Terminal%s", ":");
        else
            std::sprintf(c, "CartCentering%s", ":");
        DrawStrokeText(c, -1.9, -1.7, 0);
        glFlush();
#endif
    }
};

#endif  
