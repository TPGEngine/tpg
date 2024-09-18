#ifndef CartCentering_h
#define CartCentering_h

#include <TaskEnv.h>
#include <math.h>
#include <misc.h>
#include <stdlib.h>

#include <iostream>
#if !defined(CCANADA) && !defined(HPCC)
#include <GL/gl.h>
#include <GL/glut.h>
#endif

#define STATE_SIZE 4

/******************************************************************************/
class CartCentering : public TaskEnv {
 protected:
  /*** Parameters for simulation ***/
  const double MASSCART = 2.0;
  const double FORCE_MAG = 1.0;
  const double TAU = 0.02;  // seconds between state updates

  // these may depend on each other and dt
  const double MAX_X = 1.5;
  const double MAX_V = 6;

  const double MIN_VAR_INI = -0.75;
  const double MAX_VAR_INI = 0.75;
  const double NEAR_ORIGIN = 0.01;

  // state array indexing
  const int X = 0;
  const int V = 1;

  int lastActionD = -FORCE_MAG;
  uniform_real_distribution<> disReset;

  const int n_eval_train_ = 20;
  const int n_eval_validation_ = 0;
  const int n_eval_test_ = 100;

 public:
  /****************************************************************************/
  CartCentering() {
    disReset = uniform_real_distribution<>(MIN_VAR_INI, MAX_VAR_INI);
    actionsDiscrete.push_back(-FORCE_MAG);
    actionsDiscrete.push_back(0.0);
    actionsDiscrete.push_back(FORCE_MAG);
    eval_type_ = "Control";
    max_step = 500;
    state.reserve(STATE_SIZE);
    state.resize(STATE_SIZE);
    state_po.reserve(STATE_SIZE);
    state_po.resize(STATE_SIZE);
  }

  /****************************************************************************/
  ~CartCentering() {}

  int GetNumEval(int phase) {
    if (phase == 0)
      return n_eval_train_;
    else if (phase == 1)
      return n_eval_validation_;
    else
      return n_eval_test_;
  }

  /****************************************************************************/
  void normalizeState(bool po) {
    if (po) {
      state_po[X] /= MAX_X;
      state_po[V] /= M_PI;
    }
  }

  /****************************************************************************/
  void reset(mt19937 &rng) {
    step = 0;

    do {
      state_po[X] = state[X] = disReset(rng);
      state[V] = disReset(rng);
      terminalState = false;
    } while (terminal());

    state_po[V] = disNoise(rng);

    state[2] = disNoise(rng);
    state[3] = disNoise(rng);

    reward = 0;

    normalizeState(true);
  }

  /****************************************************************************/
  bool terminal() {
    terminalState = step >= max_step ||
                            (abs(state[X]) <= NEAR_ORIGIN &&
                             abs(state[V]) <= NEAR_ORIGIN) ||
                            abs(state[X]) > MAX_X
                        ? true
                        : false;
    return terminalState;
  }

  /****************************************************************************/
  Results update(int actionD, double actionC, mt19937 &rng) {
    (void)actionC;
    double force;

    // action 1 is ignored
    if (actionD == 0) {
      lastActionD = 0;
      force = actionsDiscrete[lastActionD];
    } else if (actionD == 2) {
      lastActionD = 2;
      force = actionsDiscrete[lastActionD];
    } else
      force = actionsDiscrete[lastActionD];

    double acc_t = force / MASSCART;

    state[X] += TAU * state[V];
    state_po[X] = state[X];

    state[V] += TAU * acc_t;
    state[V] = bound(state[V], -MAX_V, MAX_V);
    state_po[V] = disNoise(rng);

    state[2] = disNoise(rng);
    state[3] = disNoise(rng);

    step++;

    if (terminal())
      reward = -((((abs(state[X]) / MAX_X) + (abs(state[V]) / MAX_V) / 2)) +
                 (((double)step / max_step) * 0.1));
    else
      reward = 0;

    normalizeState(true);

    return {reward, 0.0};
  }

  /****************************************************************************/
  // opengl
  void display_function(int episode, int actionD, double actionC) {
    (void)episode;
    (void)actionD;
    (void)actionC;
#if !defined(CCANADA) && !defined(HPCC)
    glClear(GL_COLOR_BUFFER_BIT);

    glLineWidth(5.0);

    // cart
    glColor3f(0.0, 0.0, 1.0);
    glBegin(GL_TRIANGLES);
    glVertex2f(state[X] - 0.15, 0.075);
    glVertex2f(state[X] - 0.15, -0.075);
    glVertex2f(state[X] + 0.15, 0.075);
    glVertex2f(state[X] + 0.15, 0.075);
    glVertex2f(state[X] - 0.15, -0.075);
    glVertex2f(state[X] + 0.15, -0.075);
    glEnd();
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_LINES);
    glVertex2d(state[X], -0.075);
    glVertex2d(state[X], 0.075);

    // x bounds surface
    glVertex2d(-MAX_X, 0.0);
    glVertex2d(MAX_X, 0.0);

    // ticks (green centre) TMP
    for (int i = 1; MAX_X - (i * 0.1) > -MAX_X; i++) {
      glVertex2d(MAX_X - (i * 0.1), 0.0);
      if (isEqual(MAX_X - (i * 0.1), 0)) {
        glColor3f(0.0, 1.0, 0.0);
        glVertex2d(1.5 - (i * 0.1),
                   isEqual(1.5 - (i * 0.1), 0) ? -0.1 : -0.025);
        glColor3f(1.0, 1.0, 1.0);
      }
    }
    glEnd();

    if (step > 0) {
      // discrete action arrows, action 1 is ignored
      double force = 0;
      if (actionD == 0)
        force = actionsDiscrete[0];
      else if (actionD == 2)
        force = actionsDiscrete[2];
      else
        force = actionsDiscrete[lastActionD];

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

      if (abs(state[X]) <= NEAR_ORIGIN && abs(state[V]) <= NEAR_ORIGIN) {
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
      drawTrace(0, "Action:", force / FORCE_MAG, -1.0);
    }

    glColor3f(1.0, 1.0, 1.0);
    glLineWidth(1.0);
    drawEpisodeStepCounter(episode, step, -1.9, -1.9);

    char c[80];
    if (step == 0)
      sprintf(c, "CartCentering Initial Conditions%s", ":");
    else if (terminal())
      sprintf(c, "CartCentering Terminal%s", ":");
    else
      sprintf(c, "CartCentering%s", ":");
    drawStrokeText(c, -1.9, -1.7, 0);

    glFlush();
#endif
  }
};

#endif
