#ifndef cartpole_h
#define cartpole_h

#include <TaskEnv.h>
#include <math.h>
#include <stdlib.h>

#include <iostream>
#if !defined(CCANADA) && !defined(HPCC)
#include <GL/gl.h>
#include <GL/glut.h>
#endif

#define STATE_SIZE 4

using namespace std;

class CartPole : public TaskEnv {
 protected:
  const double GRAVITY = 9.8;
  const double MASSCART = 1.0;
  const double MASSPOLE = 0.1;
  const double TOTAL_MASS = (MASSPOLE + MASSCART);
  const double LENGTH = 0.5; /* actually half the pole's length */
  const double POLEMASS_LENGTH = (MASSPOLE * LENGTH);
  const double FORCE_MAG = 10.0;
  const double TAU = 0.02; /* seconds between state updates */
  const double FOURTHIRDS = 1.3333333333333;
  const double SIX_DEGREES = 0.1047198;
  const double SEVEN_DEGREES = 0.1221730;
  const double TEN_DEGREES = 0.1745329;
  const double TWELVE_DEGREES = 0.2094384;
  const double FIFTEEN_DEGREES = 0.2617993;
  const double TWELVE_DEGREES_SQR = TWELVE_DEGREES * TWELVE_DEGREES;

  const double MIN_X = -1;
  const double MAX_X = 1;

  // state array indexing
  const int _x = 0;
  const int _theta = 1;
  const int _x_dot = 2;
  const int _theta_dot = 3;

  int lastActionD = 0;

  const int n_eval_train_ = 20;
  const int n_eval_validation_ = 0;
  const int n_eval_test_ = 100;

 public:
  CartPole() {
    disReset = uniform_real_distribution<>(-0.05, 0.05);
    actionsDiscrete.push_back(-FORCE_MAG);
    actionsDiscrete.push_back(0.0);
    actionsDiscrete.push_back(FORCE_MAG);
    eval_type_ = "Control";
    max_step = 500;
    state.reserve(STATE_SIZE);
    state.resize(STATE_SIZE);
    state_po.reserve(STATE_SIZE - 2);
    state_po.resize(STATE_SIZE - 2);
  }

  ~CartPole() {
    state.clear();
    state_po.clear();
    actionsDiscrete.clear();
    actionTrace.clear();
  }

  int GetNumEval(int phase) {
    if (phase == 0)
      return n_eval_train_;
    else if (phase == 1)
      return n_eval_validation_;
    else
      return n_eval_test_;
  }

  void normalizeState(bool po) {
    if (po) {
      state_po[_x] /= MAX_X;
      state_po[_theta] /= TWELVE_DEGREES;
    }
  }

  void reset(mt19937 &rng) {
    state_po[_x] = state[_x] = disReset(rng);
    state_po[_theta] = state[_theta] = disReset(rng);
    state[_x_dot] = disReset(rng);
    state[_theta_dot] = disReset(rng);
    reward = 0;
    step = 0;
    terminalState = false;
    normalizeState(true);
  }

  bool terminal() {
    if (step >= max_step || abs(state[_theta]) > TWELVE_DEGREES ||
        abs(state[_x]) > MAX_X)
      terminalState = true;
    return terminalState;
  }

  Results update(int actionD, double actionC, mt19937 &rng) {
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

    costheta = cos(state[_theta]);
    sintheta = sin(state[_theta]);

    temp = (force + POLEMASS_LENGTH * state[_theta_dot] * state[_theta_dot] *
                        sintheta) /
           TOTAL_MASS;

    thetaacc =
        (GRAVITY * sintheta - costheta * temp) /
        (LENGTH * (FOURTHIRDS - MASSPOLE * costheta * costheta / TOTAL_MASS));

    xacc = temp - POLEMASS_LENGTH * thetaacc * costheta / TOTAL_MASS;

    /*** Update the four state variables, using Euler's method. ***/

    state[_x] += TAU * state[_x_dot];
    state_po[_x] = state[_x];

    state[_x_dot] += TAU * xacc;

    state[_theta] += TAU * state[_theta_dot];
    state_po[_theta] = state[_theta];

    state[_theta_dot] += TAU * thetaacc;

    step++;

    reward = 1.0;

    normalizeState(true);
    return {reward, 0.0};
  }

  // opengl
  void display_function(int episode, int actionD, double actionC) {
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
    glVertex2f(state[_x] - 0.15, 0.075);
    glVertex2f(state[_x] - 0.15, -0.075);
    glVertex2f(state[_x] + 0.15, 0.075);
    glVertex2f(state[_x] + 0.15, 0.075);
    glVertex2f(state[_x] - 0.15, -0.075);
    glVertex2f(state[_x] + 0.15, -0.075);
    glEnd();

    // pole
    x2 = state[_x] + r1 * cos(M_PI / 2 - state[_theta]);
    y2 = r1 * sin(M_PI / 2 - state[_theta]);
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_LINES);
    glVertex2d(state[_x], 0.0);
    glVertex2d(x2, y2);

    // x bounds surface
    glVertex2d(MIN_X, 0.0);
    glVertex2d(MAX_X, 0.0);

    glEnd();

    // discrete action arrows
    if (step > 0) {
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
      drawTrace(0, "Action:", force / FORCE_MAG, -1.0);
    }

    glLineWidth(1.0);
    drawEpisodeStepCounter(episode, step, -1.9, -1.9);

    glColor3f(1.0, 1.0, 1.0);
    char c[80];
    if (step == 0)
      sprintf(c, "CartPole Initial Conditions%s", ":");
    else if (terminal())
      sprintf(c, "CartPole Terminal%s", ":");
    else
      sprintf(c, "CartPole%s", ":");

    drawStrokeText(c, -1.9, -1.7, 0);

    glFlush();
#endif
  }
};

#endif