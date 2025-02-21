#ifndef phylogenetic_fitness_h
#define phylogenetic_fitness_h

#include<TPG.h>
#include "tpg_eval_mpi.h"

// Returns the fitness of a team on a given task using its phylogeny
inline double estimate_fitness(TPG &tpg, team *tm, int task) {
  std::vector<long> visited = {tm->id_};
  list<long> queue = {tm->id_};

  // Breadth-first search through phylogeny
  while (!queue.empty()) {
    long currId = queue.front();
    queue.pop_front();

    // If the team has been evaluated on the task, return its fitness
    vector<double> taskFitnesses = tpg.phylo_graph_[currId].taskFitnesses;
    int sizeInt = static_cast<int>(taskFitnesses.size());
    if (task < sizeInt) {
      return taskFitnesses[task];
    }

    for (long ancId : tpg.phylo_graph_[currId].ancestorIds) {
      if (std::find(visited.begin(), visited.end(), ancId) == visited.end()) {
        visited.push_back(ancId);
        queue.push_back(ancId);
      }
    }
  }

  cerr << "Reached unexpected point in estimate_fitness function" << endl;
  return 0;
}

/// @brief Estimates the fitness of all teams on a given set of tasks
/// @param tpg The TPG instance with all the teams
/// @param tasks The set of all tasks in the TPG
/// @param estTasks Task indices to estimate fitness on
inline void estimate_main(TPG &tpg, vector<TaskEnv *> &tasks, vector<int> estTasks) {
  // Loop through tasks
  for (int task : estTasks) {
    tpg.state_["active_task"] = task;
    auto teams_to_eval = GetTeamsToEval(tpg, tasks[task]);

    // Loop through teams
    for (auto tm : teams_to_eval) {
      // Estimate fitness of team
      double est_fit = estimate_fitness(tpg, tm, task);

      string fingerprint = "";
      vector<double> r_stats_double(4);
      r_stats_double[0] = est_fit;
      vector<int> r_stats_int(4);
      r_stats_int[POINT_AUX_INT_TASK] = task;
      r_stats_int[POINT_AUX_INT_PHASE] = tpg.GetState("phase");

      for (int i = 0; i < tasks[task]->GetNumEval(tpg.GetState("phase")); i++) {
        r_stats_int[POINT_AUX_INT_ENVSEED] = i;
        tpg.setOutcome(tm, fingerprint, r_stats_double, r_stats_int,
                       tpg.GetState("t_current"));
      }
    }
  }
}

#endif