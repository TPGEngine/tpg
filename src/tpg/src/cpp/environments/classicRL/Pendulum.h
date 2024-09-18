#ifndef Pendulum_h
#define Pendulum_h

#include <TaskEnv.h>
#include <math.h>
#include <stdlib.h>

#include <iostream>

#if !defined(CCANADA) && !defined(HPCC)
#include <GL/gl.h>
#include <GL/glut.h>
#endif

#define PENDULUM_STATE_SIZE 3
#define PENDULUM_DIM 2

using namespace std;

class Pendulum : public TaskEnv {
 protected:
  const double maxSpeed = 8;
  const double maxTorque = 2;
  double max_costs;
  const double m = 1.0;
  const double l = 1.0;
  const double g = 10;
  const double dt = 0.05;
  uniform_real_distribution<> disResetDot;

  // internal state differs from state for observation
  vector<double> _state;
  // state array indexing
  const int _theta = 0;
  const int _thetaDot = 1;

  double costs_sum;

  const int n_eval_train_ = 20;
  const int n_eval_validation_ = 0;
  const int n_eval_test_ = 100;

 public:
  double angle_normalize(double x) {
    double a = fmod(x + M_PI, 2 * M_PI);
    return a >= 0 ? (a - M_PI) : (a + M_PI);
  }

  bool discreteActions() const { return false; }

  double theta() { return _state[_theta]; }

  double thetaDot() { return _state[_thetaDot]; }

  double maxActionContinuous() const { return maxTorque; }

  double minActionContinuous() const { return -maxTorque; }

  Pendulum() {
    disReset = uniform_real_distribution<>(-M_PI, M_PI);
    disResetDot = uniform_real_distribution<>(-1.0, 1.0);
    actionsDiscrete.push_back(-maxTorque);
    actionsDiscrete.push_back(0.0);
    actionsDiscrete.push_back(maxTorque);
    eval_type_ = "Control";
    max_step = 300;
    _state.reserve(PENDULUM_DIM);
    _state.resize(PENDULUM_DIM);

    max_costs =
        pow(M_PI, 2) + 0.1 * pow(maxSpeed, 2) + 0.001 * pow(maxTorque, 2);
    // max_costs_all = -(max_costs * max_step);
    state.reserve(PENDULUM_STATE_SIZE);
    state.resize(PENDULUM_STATE_SIZE);
    state_po.reserve(PENDULUM_STATE_SIZE - 1);
    state_po.resize(PENDULUM_STATE_SIZE - 1);
  }

  ~Pendulum() {}

  int GetNumEval(int phase) {
    if (phase == 0)
      return n_eval_train_;
    else if (phase == 1)
      return n_eval_validation_;
    else
      return n_eval_test_;
  }

  void reset(mt19937 &rng) {
    _state[_theta] = disReset(rng);
    _state[_thetaDot] = disResetDot(rng);

    state[0] = state_po[0] = cos(_state[_theta]);
    state[1] = state_po[1] = sin(_state[_theta]);

    state[2] = _state[_thetaDot];

    // state[3] = disNoise(rng);

    reward = 0;

    step = 0;
    terminalState = false;
  }

  bool terminal() {
    terminalState = step >= max_step ? true : false;
    return terminalState;
  }

  Results update(int actionD, double actionC, mt19937 &rng) {
    (void)actionD;
    double torque = bound(actionC, -maxTorque, maxTorque);

    double costs = pow(angle_normalize(_state[_theta]), 2) +
                   0.1 * pow(_state[_thetaDot], 2) + 0.001 * pow(torque, 2);
    double newThetaDot =
        _state[_thetaDot] + (-3 * g / (2 * l) * sin(_state[_theta] + M_PI) +
                             3.0 / (m * pow(l, 2)) * torque) *
                                dt;
    _state[_theta] = _state[_theta] + newThetaDot * dt;
    _state[_thetaDot] = newThetaDot;
    _state[_thetaDot] = bound(_state[_thetaDot], -maxSpeed, maxSpeed);

    state[0] = state_po[0] = cos(_state[_theta]);
    state[1] = state_po[1] = sin(_state[_theta]);

    state[2] = _state[_thetaDot];

    // state[3] = disNoise(rng);

    step++;

    reward = -costs;

    return {reward, 0.0};
  }

  // opengl
  void display_function(int episode, int actionD, double actionC) {
    (void)episode;
    (void)actionD;
    (void)actionC;
#if !defined(CCANADA) && !defined(HPCC)
    double r1 = 1.0;
    double x2, y2;
    glClear(GL_COLOR_BUFFER_BIT);

    glLineWidth(5.0);

    x2 = r1 * cos(M_PI / 2 - _state[_theta]);
    y2 = r1 * sin(M_PI / 2 - _state[_theta]);
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_LINES);
    glVertex2d(0.0, 0.0);
    glVertex2d(-x2, y2);

    // surface
    glVertex2d(-1.5, 0.0);
    glVertex2d(1.5, 0.0);
    glEnd();

    if (step > 0) {
      // action
      double torque = bound(actionC, -maxTorque, maxTorque);
      const int sides = 40;
      const double radius = 0.2;

      int dir = 1;
      int start = 0;
      int end = 0;

      if (actionC < 0) {
        dir = -1;
        start = -180;
        end = -90;
      } else if (actionC > 0) {
        dir = 1;
        start = -90;
        end = 20;
      }
      glBegin(GL_LINES);
      for (int a = start; a < end; a += 360 / sides) {
        double heading = a * M_PI / 180;
        glVertex2d(cos(heading) * radius, sin(heading) * radius);
      }
      glEnd();

      // arrow
      glBegin(GL_POLYGON);
      glVertex3f(dir * 0.25, 0.0, 0);
      glVertex3f(dir * 0.2, 0.1 + ((abs(torque) / maxTorque) / 10), 0);
      glVertex3f(dir * 0.15, 0.0, 0);
      glEnd();

      // action trace
      glLineWidth(2.0);
      drawTrace(0, "Action:", torque / maxTorque, -1.2);
    }
    glColor3f(1.0, 1.0, 1.0);
    glLineWidth(1.0);
    drawEpisodeStepCounter(episode, step, -1.9, -1.9);

    char c[80];
    if (step == 0)
      sprintf(c, "Pendulum Initial Conditions%s", ":");
    else if (terminal())
      sprintf(c, "Pendulum Terminal%s", ":");
    else
      sprintf(c, "Pendulum%s", ":");
    drawStrokeText(c, -1.9, -1.7, 0);

    glFlush();
#endif
  }
};
#endif
