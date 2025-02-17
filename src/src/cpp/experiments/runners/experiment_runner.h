#ifndef I_EXPERIMENT_RUNNER_H
#define I_EXPERIMENT_RUNNER_H

#include "tpg_eval_mpi.h"
#include "phylogenetic_fitness.h"
#include <misc.h>
#include <chrono>
#include <cstdlib>
#include <algorithm>
#include <any>
#include "evaluators_mujoco.h"
#include "TPG.h"
#include <boost/mpi.hpp>


class ExperimentRunner {
 public:
  virtual ~ExperimentRunner() = default;
  virtual void run() = 0;
};

#endif
