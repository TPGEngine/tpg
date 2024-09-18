#ifndef MountainCarContinuous_h
#define MountainCarContinuous_h

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

class MountainCarContinuous : public TaskEnv {
 protected:
  const double min_action = -1.0;
  const double max_action = 1.0;
  const double min_position = -1.2;
  const double max_position = 0.6;
  const double max_speed = 0.07;
  const double goal_position = 0.45;
  const double goal_velocity = 0.0;
  const double power = 0.0015;
  const double gravity = 0.0025;

  // state array indexing
  const int _position = 0;
  const int _velocity = 1;

  double min_reward;

  const int n_eval_train_ = 20;
  const int n_eval_validation_ = 0;
  const int n_eval_test_ = 100;

 public:
  bool discreteActions() const { return false; }
  double maxActionContinuous() const { return max_action; }
  double minActionContinuous() const { return min_action; }

  MountainCarContinuous() {
    disReset = uniform_real_distribution<>(-0.6, -0.4);
    eval_type_ = "Control";
    max_step = 200;
    state.reserve(STATE_SIZE);
    state.resize(STATE_SIZE);
    state_po.reserve(STATE_SIZE);
    state_po.resize(STATE_SIZE);
  }

  ~MountainCarContinuous() {}

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
      state_po[_position] =
          (state_po[_position] - min_position) / (max_position - min_position);
    }
  }

  void reset(mt19937 &rng) {
    state[_position] = state_po[_position] = disReset(rng);
    state[_velocity] = 0;

    state_po[_velocity] = disNoise(rng);

    state[2] = disNoise(rng);
    state[3] = disNoise(rng);

    reward = 0;

    step = 0;
    terminalState = false;
    normalizeState(true);
  }

  bool terminal() {
    if (step >= max_step || (state[_position] >= goal_position &&
                             state[_velocity] >= goal_velocity))
      terminalState = true;
    return terminalState;
  }

  Results update(int actionD, double actionC, mt19937 &rng) {
    (void)actionD;
    double force = bound(actionC, min_action, max_action);
    state[_velocity] += force * power - gravity * cos(3 * state[_position]);
    state[_velocity] = bound(state[_velocity], -max_speed, max_speed);
    state[_position] += state[_velocity];
    state[_position] = bound(state[_position], min_position, max_position);
    if (state[_position] == min_position && state[_velocity] < 0)
      state[_velocity] = 0;

    state_po[_position] = state[_position];
    state_po[_velocity] = disNoise(rng);

    state[2] = disNoise(rng);
    state[3] = disNoise(rng);

    step++;

    // reward 2
    if (terminal() && step < max_step)
      reward = 100;
    else
      reward = -(pow(force, 2) * 0.1);

    normalizeState(true);
    return {reward, 0.0};
  }

  // opengl
  void display_function(int episode, int actionD, double actionC) {
    (void)episode;
    (void)actionD;
    (void)actionC;
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
    vector<double> xs = linspace(min_position, max_position, 100);
    for (size_t i = 1; i < xs.size() - 1; i++) {
      glVertex2d(x, sin(3 * xs[i]) * .45 + .55);
      if (state[_position] >= xs[i - 1] && state[_position] <= xs[i + 1]) {
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

    if (step > 0) {
      double force = bound(actionC, min_action, max_action);
      glLineWidth(2.0);
      drawTrace(0, "Action:", force, -1.0);
    }

    glColor3f(1.0, 1.0, 1.0);
    glLineWidth(1.0);
    drawEpisodeStepCounter(episode, step, -1.9, -1.9);

    char c[80];
    if (step == 0)
      sprintf(c, "MountainCarContinuous Initial Conditions%s", ":");
    else if (terminal())
      sprintf(c, "MountainCarContinuous Terminal%s", ":");
    else
      sprintf(c, "Mountain Car%s", ":");
    drawStrokeText(c, -1.9, -1.7, 0);

    glFlush();
#endif
  }
};

#endif
