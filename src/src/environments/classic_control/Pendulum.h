#ifndef Pendulum_h
#define Pendulum_h

#include <math.h>
#include <stdlib.h>
#include <iostream>

#include "ClassicControlEnv.h"

#if !defined(CCANADA) && !defined(HPCC)
#include <GL/gl.h>
#include <GL/glut.h>
#endif

// Pendulum swing-up control environment
constexpr int kPendulumStateSize = 3;
constexpr int kPendulumDimensions = 2;

class Pendulum : public ClassicControlEnv {
   protected:
    // Environment constants
    static constexpr double kMaxSpeed = 8.0;
    static constexpr double kMaxTorque = 2.0;
    static constexpr double kMass = 1.0;
    static constexpr double kLength = 1.0;
    static constexpr double kGravity = 10.0;
    static constexpr double kTimeStep = 0.05;

    // State array indices
    enum StateIndex {
        kTheta = 0,
        kThetaDot = 1
    };

    enum StateObservationIndex {
        kCosTheta = 0,
        kSinTheta = 1,
        kThetaDotObs = 2
    };

    // Internal state differs from state for observation
    std::vector<double> internal_state_;
    std::uniform_real_distribution<> reset_dot_distribution_;
    double max_costs_;
    double costs_sum_;

   public:
    double AngleNormalize(double x) {
        double a = fmod(x + M_PI, 2 * M_PI);
        return a >= 0 ? (a - M_PI) : (a + M_PI);
    }

    bool DiscreteActions() const override { return false; }
    double MaxActionContinuous() const override { return kMaxTorque; }
    double MinActionContinuous() const override { return -kMaxTorque; }

    double theta() { return internal_state_[StateIndex::kTheta]; }
    double thetaDot() { return internal_state_[StateIndex::kThetaDot]; }

    Pendulum() {
        n_eval_train_ = 20;
        n_eval_validation_ = 0;
        n_eval_test_ = 100;
        dis_reset = std::uniform_real_distribution<>(-M_PI, M_PI);
        reset_dot_distribution_ = std::uniform_real_distribution<>(-1.0, 1.0);
        actionsDiscrete.push_back(-kMaxTorque);
        actionsDiscrete.push_back(0.0);
        actionsDiscrete.push_back(kMaxTorque);
        eval_type_ = "Control";
        max_step_ = 300;
        internal_state_.reserve(kPendulumDimensions);
        internal_state_.resize(kPendulumDimensions);

        max_costs_ = pow(M_PI, 2) + 0.1 * pow(kMaxSpeed, 2) + 
                    0.001 * pow(kMaxTorque, 2);
        state_.reserve(kPendulumStateSize);
        state_.resize(kPendulumStateSize);
        state_po_.reserve(kPendulumStateSize - 1);
        state_po_.resize(kPendulumStateSize - 1);
    }

    //! Resets the pendulum to a initial state based on specified Bounds
    void Reset(std::mt19937& rng) override {
        internal_state_[StateIndex::kTheta] = dis_reset(rng);
        internal_state_[StateIndex::kThetaDot] = reset_dot_distribution_(rng);

        state_[StateObservationIndex::kCosTheta] = state_po_[StateObservationIndex::kCosTheta] = cos(internal_state_[StateIndex::kTheta]);
        state_[StateObservationIndex::kSinTheta] = state_po_[StateObservationIndex::kSinTheta] = sin(internal_state_[StateIndex::kTheta]);
        state_[StateObservationIndex::kThetaDotObs] = internal_state_[StateIndex::kThetaDot];

        reward = 0;
        step_ = 0;
        terminalState = false;
    }

    //! Checks if the current state is terminal based on steps
    bool Terminal() override {
        terminalState = step_ >= max_step_;
        return terminalState;
    }

    //! Updates the pendulum state based on the given action
    Results Update(int action_discrete, double action_continuous, 
                  std::mt19937& rng) override {
        (void)action_discrete;
        double torque = Bound(action_continuous, -kMaxTorque, kMaxTorque);

        double costs = pow(AngleNormalize(internal_state_[StateIndex::kTheta]), 2) +
                      0.1 * pow(internal_state_[StateIndex::kThetaDot], 2) + 
                      0.001 * pow(torque, 2);

        double new_theta_dot = internal_state_[StateIndex::kThetaDot] + 
            (-3 * kGravity / (2 * kLength) * 
             sin(internal_state_[StateIndex::kTheta] + M_PI) +
             3.0 / (kMass * pow(kLength, 2)) * torque) * kTimeStep;

        internal_state_[StateIndex::kTheta] += new_theta_dot * kTimeStep;
        internal_state_[StateIndex::kThetaDot] = Bound(new_theta_dot, -kMaxSpeed, kMaxSpeed);

        state_[StateObservationIndex::kCosTheta] = state_po_[StateObservationIndex::kCosTheta] = cos(internal_state_[StateIndex::kTheta]);
        state_[StateObservationIndex::kSinTheta] = state_po_[StateObservationIndex::kSinTheta] = sin(internal_state_[StateIndex::kTheta]);
        state_[StateObservationIndex::kThetaDotObs] = internal_state_[StateIndex::kThetaDot];

        step_++;
        reward = -costs;

        return {reward, 0.0};
    }

    //! Displays the pendulum state using OpenGL 
    void DisplayFunction(int episode, int action_discrete, 
                         double action_continuous) {
        (void)episode;
        (void)action_discrete;
        (void)action_continuous;
#if !defined(CCANADA) && !defined(HPCC)
        double radius = 1.0;
        double x2, y2;
        glClear(GL_COLOR_BUFFER_BIT);

        glLineWidth(5.0);

        x2 = radius * cos(M_PI / 2 - internal_state_[StateIndex::kTheta]);
        y2 = radius * sin(M_PI / 2 - internal_state_[StateIndex::kTheta]);
        glColor3f(1.0, 1.0, 1.0);
        glBegin(GL_LINES);
        glVertex2d(0.0, 0.0);
        glVertex2d(-x2, y2);

        // Surface
        glVertex2d(-1.5, 0.0);
        glVertex2d(1.5, 0.0);
        glEnd();

        if (step_ > 0) {
            // Action visualization
            double torque = Bound(action_continuous, -kMaxTorque, kMaxTorque);
            const int sides = 40;
            const double arc_radius = 0.2;

            int direction = 1;
            int start_angle = 0;
            int end_angle = 0;

            if (action_continuous < 0) {
                direction = -1;
                start_angle = -180;
                end_angle = -90;
            } else if (action_continuous > 0) {
                direction = 1;
                start_angle = -90;
                end_angle = 20;
            }

            glBegin(GL_LINES);
            for (int angle = start_angle; angle < end_angle; angle += 360 / sides) {
                double heading = angle * M_PI / 180;
                glVertex2d(cos(heading) * arc_radius, sin(heading) * arc_radius);
            }
            glEnd();

            // Arrow
            glBegin(GL_POLYGON);
            glVertex3f(direction * 0.25, 0.0, 0);
            glVertex3f(direction * 0.2, 
                      0.1 + ((std::abs(torque) / kMaxTorque) / 10), 0);
            glVertex3f(direction * 0.15, 0.0, 0);
            glEnd();

            // Action trace
            glLineWidth(2.0);
            DrawTrace(0, "Action:", torque / kMaxTorque, -1.2);
        }

        glColor3f(1.0, 1.0, 1.0);
        glLineWidth(1.0);
        DrawEpisodeStepCounter(episode, step_, -1.9, -1.9);

        char text[80];
        if (step_ == 0)
            sprintf(text, "Pendulum Initial Conditions%s", ":");
        else if (Terminal())
            sprintf(text, "Pendulum Terminal%s", ":");
        else
            sprintf(text, "Pendulum%s", ":");
        DrawStrokeText(text, -1.9, -1.7, 0);

        glFlush();
#endif
    }
};
#endif
