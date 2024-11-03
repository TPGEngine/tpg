#include "TPG.h"

/******************************************************************************/
TPG::TPG() {
   instruction::SetupOps();
   real_dist_ = uniform_real_distribution<>(0.0, 1.0);
   _Memids.resize(memoryEigen::NUM_MEMORY_TYPES);
   _Memory.resize(memoryEigen::NUM_MEMORY_TYPES);
   for (size_t i = 0; i < _NUM_PHASE; i++) _numEliteTeamsCurrent.push_back(0);
   _ops.resize(instruction::NUM_OP);
   fill(_ops.begin(), _ops.end(), false);
   rngs_.resize(NUM_RNG);
   seeds_.resize(NUM_RNG);
}

/******************************************************************************/
TPG::~TPG() {}

/******************************************************************************/
void TPG::AddProgram(program *p) {
   _L[p->id_] = p;
   _Lids.push_back(p->id_);
}

/******************************************************************************/
void TPG::removeProgram(program *p, bool updateLids) {
   if (updateLids) {
      auto it = find(_Lids.begin(), _Lids.end(), p->id_);
      if (it == _Lids.end())
         die(__FILE__, __FUNCTION__, __LINE__, "failed to remove program");
      swap(_Lids[distance(_Lids.begin(), it)], _Lids.back());
      _Lids.pop_back();
   }
   _L.erase(p->id_);
}

/******************************************************************************/
void TPG::AddTeam(team *tm) {
   _M.insert(tm);
   if (tm->root_) {
      _Mroot.insert(tm);
   }
   _teamMap[tm->id_] = tm;
}

/******************************************************************************/
void TPG::RemoveTeam(team *tm, deque<program *> &p) {
   // decrement program refs
   for (auto prog : tm->members_) {
      prog->nrefs_--;
      if (prog->nrefs_ < 1) p.push_back(prog);
   }
   // TODO(skelly): test cloning
   if (_teamMap.find(tm->cloneId_) != _teamMap.end())
      _teamMap[tm->cloneId_]->clones_--;
   _teamMap.erase(tm->id_);
   _M.erase(tm);
   delete tm;
}

/******************************************************************************/
void TPG::AddMemory(memoryEigen *m) {
   _Memory[m->type_][m->id_] = m;
   _Memids[m->type_].push_back(m->id_);
}

/******************************************************************************/
void TPG::removeMemory(memoryEigen *m) {
   auto it = find(_Memids[m->type_].begin(), _Memids[m->type_].end(), m->id_);
   if (it == _Memids[m->type_].end())
      die(__FILE__, __FUNCTION__, __LINE__, "failed to remove memoryEigen");
   swap(_Memids[m->type_][it - _Memids[m->type_].begin()],
        _Memids[m->type_].back());
   _Memids[m->type_].pop_back();
   _Memory[m->type_].erase(m->id_);
}

/******************************************************************************/
team *TPG::getTeamByID(long id) {
   for (auto teiter = _M.begin(); teiter != _M.end(); teiter++)
      if ((*teiter)->id_ == id) return *teiter;
   return *(_M.begin());
}

/******************************************************************************/
bool TPG::haveEliteTeam(string taskset, int fitMode, int phase) {
   return _eliteTeamPS[taskset][fitMode].find(phase) !=
          _eliteTeamPS[taskset][fitMode].end();
}

/******************************************************************************/
void TPG::Seed(size_t i, uint_fast32_t s) {
   seeds_[i] = s;
   rngs_[i].seed(seeds_[i]);
}

void TPG::InitExperimentTracking(APIClient *apiClient) {
   api_client_ = apiClient;
}

/******************************************************************************/
void TPG::clearMemory() {
   for (int mem_t = 0; mem_t < memoryEigen::NUM_MEMORY_TYPES; mem_t++) {
      for (auto meiter = _Memory[mem_t].begin(); meiter != _Memory[mem_t].end();
           meiter++) {
         meiter->second->ClearWorking();
         meiter->second->ClearReadTime();
         meiter->second->ClearWriteTime();
      }
   }
}

/******************************************************************************/
program *TPG::getAction(team *tm, state *s, bool updateActive,
                        set<team *, teamIdComp> &visitedTeams,
                        long &decisionInstructions, int timeStep,
                        vector<team *> &teamPath, mt19937 &rng, bool verbose) {
   visitedTeams.clear();
   decisionInstructions = 0;
   teamPath.clear();
   return tm->getAction(s, _teamMap, updateActive, visitedTeams,
                        decisionInstructions, timeStep, teamPath, rng, verbose);
}

/******************************************************************************/
program *TPG::getAction(
    team *tm, state *s, bool updateActive,
    set<team *, teamIdComp> &visitedTeams, long &decisionInstructions,
    int timeStep, vector<program *> &allPrograms,
    vector<program *> &winningPrograms, vector<set<long>> &decisionFeatures,
    // vector<set<memoryEigen *, memoryEigenIdComp>> &decisionMemories,
    vector<team *> &teamPath, mt19937 &rng, bool verbose) {
   allPrograms.clear();
   winningPrograms.clear();
   decisionInstructions = 0;
   decisionFeatures.clear();
   // decisionMemories.clear();
   // visitedTeams.clear();
   teamPath.clear();
   return tm->getAction(
       s, _teamMap, updateActive, visitedTeams, decisionInstructions, timeStep,
       allPrograms, winningPrograms, decisionFeatures, teamPath, rng, verbose);
}

/******************************************************************************/
void TPG::GetAllNodes(team *tm, set<team *, teamIdComp> &teams,
                      set<program *, programIdComp> &programs) {
   teams.clear();
   programs.clear();
   tm->GetAllNodes(_teamMap, teams, programs);
}

// /******************************************************************************/
// void TPG::GetAllNodes(team *tm, set<team *, teamIdComp> &teams,
//                       set<program *, programIdComp> &programs,
//                       set<memoryEigen *, memoryEigenIdComp> &memories) {
//   teams.clear();
//   programs.clear();
//   memories.clear();
//   tm->GetAllNodes(_teamMap, teams, programs, memories);
// }

/******************************************************************************/
void TPG::getTeams(vector<team *> &t, bool roots) const {
   if (roots)
      t.assign(_Mroot.begin(), _Mroot.end());
   else
      t.assign(_M.begin(), _M.end());
}

// /******************************************************************************/
// void TPG::getTeams(map<long, team *> &t, bool roots) const {
//   t.clear();
//   if (roots)
//     for (auto teiter = _Mroot.begin(); teiter != _Mroot.end(); teiter++)
//       t[(*teiter)->id_] = *teiter;
//   else
//     for (auto teiter = _M.begin(); teiter != _M.end(); teiter++)
//       t[(*teiter)->id_] = *teiter;
// }

/******************************************************************************/
map<long, team *> TPG::GetTeamsInMap(bool roots) const {
   map<long, team *> t;
   if (roots)
      for (auto teiter = _Mroot.begin(); teiter != _Mroot.end(); teiter++)
         t[(*teiter)->id_] = *teiter;
   else
      for (auto teiter = _M.begin(); teiter != _M.end(); teiter++)
         t[(*teiter)->id_] = *teiter;
   return t;
}

/******************************************************************************/
vector<team *> TPG::GetTeamsInVec(bool roots) const {
   vector<team *> teams;
   if (roots)
      for (auto teiter = _Mroot.begin(); teiter != _Mroot.end(); teiter++)
         teams.push_back(*teiter);
   else
      for (auto teiter = _M.begin(); teiter != _M.end(); teiter++)
         teams.push_back(*teiter);
   return teams;
}

/******************************************************************************/
bool TPG::isElitePS(team *tm, int phase) {
   for (auto itr1 = _eliteTeamPS.begin(); itr1 != _eliteTeamPS.end();
        itr1++) {  // taskset
      for (auto itr2 = itr1->second.begin(); itr2 != itr1->second.end();
           itr2++) {  // fitmode
                      // for (auto itr3 = itr2->second.begin(); itr3 !=
                      // itr2->second.end(); itr3++)//phase
         if (itr2->second.find(phase) != itr2->second.end() &&
             itr2->second[phase]->id_ == tm->id_)
            return true;
      }
   }
   return false;
}

/******************************************************************************/
void TPG::MarkEffectiveCode() {
   for (auto prog : _L) {
      prog.second->stateful_ = GetParam<int>("stateful");
      prog.second->MarkIntrons(params_);
   }
}

/******************************************************************************/
void TPG::printOss() {
   cout << oss.str();
   oss.str("");
}

/******************************************************************************/
void TPG::printOss(ostringstream &o) {
   oss << o.str();
   o.str("");
}

/******************************************************************************/
void TPG::ReadParameters(string file_name,
                         std::unordered_map<string, std::any> &params) {
   std::ifstream infile(file_name);
   string oneline;
   vector<string> outcome_fields;
   while (std::getline(infile, oneline)) {
      if (oneline.find('#') != std::string::npos || oneline.size() == 0)
         continue;  // skip comments and empty lines
      SplitString(oneline, ' ', outcome_fields);

      if (outcome_fields[0] == "SCALAR_SUM_OP")
         _ops[instruction::SCALAR_SUM_OP_] = true;
      if (outcome_fields[0] == "SCALAR_DIFF_OP")
         _ops[instruction::SCALAR_DIFF_OP_] = true;
      if (outcome_fields[0] == "SCALAR_PRODUCT_OP")
         _ops[instruction::SCALAR_PRODUCT_OP_] = true;
      if (outcome_fields[0] == "SCALAR_DIVISION_OP")
         _ops[instruction::SCALAR_DIVISION_OP_] = true;
      if (outcome_fields[0] == "SCALAR_ABS_OP")
         _ops[instruction::SCALAR_ABS_OP_] = true;
      if (outcome_fields[0] == "SCALAR_RECIPROCAL_OP")
         _ops[instruction::SCALAR_RECIPROCAL_OP_] = true;
      if (outcome_fields[0] == "SCALAR_SIN_OP")
         _ops[instruction::SCALAR_SIN_OP_] = true;
      if (outcome_fields[0] == "SCALAR_COS_OP")
         _ops[instruction::SCALAR_COS_OP_] = true;
      if (outcome_fields[0] == "SCALAR_TAN_OP")
         _ops[instruction::SCALAR_TAN_OP_] = true;
      if (outcome_fields[0] == "SCALAR_ARCSIN_OP")
         _ops[instruction::SCALAR_ARCSIN_OP_] = true;
      if (outcome_fields[0] == "SCALAR_ARCCOS_OP")
         _ops[instruction::SCALAR_ARCCOS_OP_] = true;
      if (outcome_fields[0] == "SCALAR_ARCTAN_OP")
         _ops[instruction::SCALAR_ARCTAN_OP_] = true;
      if (outcome_fields[0] == "SCALAR_EXP_OP")
         _ops[instruction::SCALAR_EXP_OP_] = true;
      if (outcome_fields[0] == "SCALAR_LOG_OP")
         _ops[instruction::SCALAR_LOG_OP_] = true;
      if (outcome_fields[0] == "SCALAR_HEAVYSIDE_OP")
         _ops[instruction::SCALAR_HEAVYSIDE_OP_] = true;
      if (outcome_fields[0] == "VECTOR_HEAVYSIDE_OP")
         _ops[instruction::VECTOR_HEAVYSIDE_OP_] = true;
      if (outcome_fields[0] == "MATRIX_HEAVYSIDE_OP")
         _ops[instruction::MATRIX_HEAVYSIDE_OP_] = true;
      if (outcome_fields[0] == "SCALAR_VECTOR_PRODUCT_OP")
         _ops[instruction::SCALAR_VECTOR_PRODUCT_OP_] = true;
      if (outcome_fields[0] == "SCALAR_BROADCAST_OP")
         _ops[instruction::SCALAR_BROADCAST_OP_] = true;
      if (outcome_fields[0] == "VECTOR_RECIPROCAL_OP")
         _ops[instruction::VECTOR_RECIPROCAL_OP_] = true;
      if (outcome_fields[0] == "VECTOR_NORM_OP")
         _ops[instruction::VECTOR_NORM_OP_] = true;
      if (outcome_fields[0] == "VECTOR_ABS_OP")
         _ops[instruction::VECTOR_ABS_OP_] = true;
      if (outcome_fields[0] == "VECTOR_SUM_OP")
         _ops[instruction::VECTOR_SUM_OP_] = true;
      if (outcome_fields[0] == "VECTOR_DIFF_OP")
         _ops[instruction::VECTOR_DIFF_OP_] = true;
      if (outcome_fields[0] == "VECTOR_PRODUCT_OP")
         _ops[instruction::VECTOR_PRODUCT_OP_] = true;
      if (outcome_fields[0] == "VECTOR_DIVISION_OP")
         _ops[instruction::VECTOR_DIVISION_OP_] = true;
      if (outcome_fields[0] == "VECTOR_INNER_PRODUCT_OP")
         _ops[instruction::VECTOR_INNER_PRODUCT_OP_] = true;
      if (outcome_fields[0] == "VECTOR_OUTER_PRODUCT_OP")
         _ops[instruction::VECTOR_OUTER_PRODUCT_OP_] = true;
      if (outcome_fields[0] == "SCALAR_MATRIX_PRODUCT_OP")
         _ops[instruction::SCALAR_MATRIX_PRODUCT_OP_] = true;
      if (outcome_fields[0] == "MATRIX_RECIPROCAL_OP")
         _ops[instruction::MATRIX_RECIPROCAL_OP_] = true;
      if (outcome_fields[0] == "MATRIX_VECTOR_PRODUCT_OP")
         _ops[instruction::MATRIX_VECTOR_PRODUCT_OP_] = true;
      if (outcome_fields[0] == "VECTOR_COLUMN_BROADCAST_OP")
         _ops[instruction::VECTOR_COLUMN_BROADCAST_OP_] = true;
      if (outcome_fields[0] == "VECTOR_ROW_BROADCAST_OP")
         _ops[instruction::VECTOR_ROW_BROADCAST_OP_] = true;
      if (outcome_fields[0] == "MATRIX_NORM_OP")
         _ops[instruction::MATRIX_NORM_OP_] = true;
      if (outcome_fields[0] == "MATRIX_COLUMN_NORM_OP")
         _ops[instruction::MATRIX_COLUMN_NORM_OP_] = true;
      if (outcome_fields[0] == "MATRIX_ROW_NORM_OP")
         _ops[instruction::MATRIX_ROW_NORM_OP_] = true;
      if (outcome_fields[0] == "MATRIX_TRANSPOSE_OP")
         _ops[instruction::MATRIX_TRANSPOSE_OP_] = true;
      if (outcome_fields[0] == "MATRIX_ABS_OP")
         _ops[instruction::MATRIX_ABS_OP_] = true;
      if (outcome_fields[0] == "MATRIX_SUM_OP")
         _ops[instruction::MATRIX_SUM_OP_] = true;
      if (outcome_fields[0] == "MATRIX_DIFF_OP")
         _ops[instruction::MATRIX_DIFF_OP_] = true;
      if (outcome_fields[0] == "MATRIX_PRODUCT_OP")
         _ops[instruction::MATRIX_PRODUCT_OP_] = true;
      if (outcome_fields[0] == "MATRIX_DIVISION_OP")
         _ops[instruction::MATRIX_DIVISION_OP_] = true;
      if (outcome_fields[0] == "MATRIX_MATRIX_PRODUCT_OP")
         _ops[instruction::MATRIX_MATRIX_PRODUCT_OP_] = true;
      if (outcome_fields[0] == "SCALAR_MIN_OP")
         _ops[instruction::SCALAR_MIN_OP_] = true;
      if (outcome_fields[0] == "VECTOR_MIN_OP")
         _ops[instruction::VECTOR_MIN_OP_] = true;
      if (outcome_fields[0] == "MATRIX_MIN_OP")
         _ops[instruction::MATRIX_MIN_OP_] = true;
      if (outcome_fields[0] == "SCALAR_MAX_OP")
         _ops[instruction::SCALAR_MAX_OP_] = true;
      if (outcome_fields[0] == "VECTOR_MAX_OP")
         _ops[instruction::VECTOR_MAX_OP_] = true;
      if (outcome_fields[0] == "MATRIX_MAX_OP")
         _ops[instruction::MATRIX_MAX_OP_] = true;
      if (outcome_fields[0] == "VECTOR_MEAN_OP")
         _ops[instruction::VECTOR_MEAN_OP_] = true;
      if (outcome_fields[0] == "MATRIX_MEAN_OP")
         _ops[instruction::MATRIX_MEAN_OP_] = true;
      if (outcome_fields[0] == "MATRIX_ROW_ST_DEV_OP")
         _ops[instruction::MATRIX_ROW_ST_DEV_OP_] = true;
      if (outcome_fields[0] == "VECTOR_ST_DEV_OP")
         _ops[instruction::VECTOR_ST_DEV_OP_] = true;
      if (outcome_fields[0] == "MATRIX_ST_DEV_OP")
         _ops[instruction::MATRIX_ST_DEV_OP_] = true;
      if (outcome_fields[0] == "SCALAR_CONST_SET_OP")
         _ops[instruction::SCALAR_CONST_SET_OP_] = true;
      if (outcome_fields[0] == "VECTOR_CONST_SET_OP")
         _ops[instruction::VECTOR_CONST_SET_OP_] = true;
      if (outcome_fields[0] == "MATRIX_CONST_SET_OP")
         _ops[instruction::MATRIX_CONST_SET_OP_] = true;
      if (outcome_fields[0] == "SCALAR_UNIFORM_SET_OP")
         _ops[instruction::SCALAR_UNIFORM_SET_OP_] = true;
      if (outcome_fields[0] == "VECTOR_UNIFORM_SET_OP")
         _ops[instruction::VECTOR_UNIFORM_SET_OP_] = true;
      if (outcome_fields[0] == "MATRIX_UNIFORM_SET_OP")
         _ops[instruction::MATRIX_UNIFORM_SET_OP_] = true;
      if (outcome_fields[0] == "SCALAR_GAUSSIAN_SET_OP")
         _ops[instruction::SCALAR_GAUSSIAN_SET_OP_] = true;
      if (outcome_fields[0] == "VECTOR_GAUSSIAN_SET_OP")
         _ops[instruction::VECTOR_GAUSSIAN_SET_OP_] = true;
      if (outcome_fields[0] == "MATRIX_GAUSSIAN_SET_OP")
         _ops[instruction::MATRIX_GAUSSIAN_SET_OP_] = true;
      if (outcome_fields[0] == "SCALAR_CONDITIONAL_OP")
         _ops[instruction::SCALAR_CONDITIONAL_OP_] = true;
      if (outcome_fields[0] == "SCALAR_POW_OP")
         _ops[instruction::SCALAR_POW_OP_] = true;
      if (outcome_fields[0] == "SCALAR_SQR_OP")
         _ops[instruction::SCALAR_SQR_OP_] = true;
      if (outcome_fields[0] == "SCALAR_CUBE_OP")
         _ops[instruction::SCALAR_CUBE_OP_] = true;
      if (outcome_fields[0] == "SCALAR_TANH_OP")
         _ops[instruction::SCALAR_TANH_OP_] = true;
      if (outcome_fields[0] == "SCALAR_SQRT_OP")
         _ops[instruction::SCALAR_SQRT_OP_] = true;
      if (outcome_fields[0] == "SCALAR_VECTOR_ASSIGN_OP")
         _ops[instruction::SCALAR_VECTOR_ASSIGN_OP_] = true;
      if (outcome_fields[0] == "SCALAR_MATRIX_ASSIGN_OP")
         _ops[instruction::SCALAR_MATRIX_ASSIGN_OP_] = true;
      if (outcome_fields[0] == "OBS_BUFF_SLICE_OP")
         _ops[instruction::OBS_BUFF_SLICE_OP_] = true;

      // TODO(skelly): make types part of parameter file
      // string parameters are "hard coded" here
      if (outcome_fields[0] == "active_tasks" ||
          outcome_fields[0] == "n_input" ||
          outcome_fields[0] == "n_stored_outcomes_TRAIN" ||
          outcome_fields[0] == "n_stored_outcomes_VALIDATION" ||
          outcome_fields[0] == "n_stored_outcomes_TEST" ||
          outcome_fields[0] == "forecast_fitness" ||
          outcome_fields[0] == "action_dim" ||
          outcome_fields[0] == "mj_model_path" ||
          outcome_fields[0] == "experiment_key") {
         params[outcome_fields[0]] = outcome_fields[1];
      }
      // double parameters are identified by a decimal place
      else if (outcome_fields[1].find('.') != std::string::npos) {
         params[outcome_fields[0]] = stringToDouble(outcome_fields[1]);
      }
      // otherwise we store the parameter as an integer
      else {
         params[outcome_fields[0]] = stringToInt(outcome_fields[1]);
      }
   }
}

/******************************************************************************/
void TPG::resetOutcomes(int phase, bool roots) {
   if (roots)
      for (auto teiter = _Mroot.begin(); teiter != _Mroot.end(); teiter++)
         (*teiter)->resetOutcomes(phase);
   else
      for (auto teiter = _M.begin(); teiter != _M.end(); teiter++)
         (*teiter)->resetOutcomes(phase);
}

/******************************************************************************/
void TPG::setOutcome(team *tm, string behav, vector<double> &rewards,
                     vector<int> &ints, long gtime) {
   point *p = new point(gtime, state_["point_count"]++, behav, rewards, ints);
   p->key(GetParam<int>("fit_mode"));
   tm->setOutcome(p);
}

/******************************************************************************/
void TPG::finalize() {
   // TODO(skelly): remove clears that are not required
   _allComponentsA.clear();
   _allComponentsAt.clear();
   _eliteTeams.clear();
   _eliteTeamPS.clear();
   _Mroot.clear();
   _teamMap.clear();
   _Lids.clear();
   _Memids.clear();
   _Memids.resize(memoryEigen::NUM_MEMORY_TYPES);
   state_["memory_count"] = 0;
   _numEliteTeamsCurrent.clear();
   for (size_t i = 0; i < _NUM_PHASE; i++) _numEliteTeamsCurrent.push_back(0);
   _persistenceFilterA.clear();
   _persistenceFilterA_t.clear();
   _persistenceFilterAllTime.clear();
   state_["point_count"] = 0;
   state_["program_count"] = 0;
   state_["memory_count"] = 0;
   task_set_map_.clear();

   _teamPairsToCompair.clear();

   for (auto teiter = _M.begin(); teiter != _M.end(); teiter++) {
      (*teiter)->resetOutcomes(-1);
      delete *teiter;
   }
   _M.clear();

   for (auto leiter = _L.begin(); leiter != _L.end(); leiter++)
      delete leiter->second;
   _L.clear();

   // for (int mem_t = 0; mem_t < memoryEigen::NUM_MEMORY_TYPES; mem_t++) {
   //     for (auto meiter = _Memory[mem_t].begin();
   //          meiter != _Memory[mem_t].end(); meiter++)
   //         delete meiter->second;
   //     _Memory[mem_t].clear();
   // }
   _Memory.clear();
   _Memory.resize(memoryEigen::NUM_MEMORY_TYPES);

   _phyloGraph.clear();
}

/******************************************************************************/
void TPG::TeamMutator_ProgramOrder(team *team_to_mu) {
   if ((team_to_mu)->size() > 1 &&
       real_dist_(rngs_[TPG_SEED]) < GetParam<double>("pmw")) {
      int i, j;
      uniform_int_distribution<int> dis_team_size(0, (team_to_mu)->size() - 1);
      i = dis_team_size(rngs_[TPG_SEED]);
      do {
         j = dis_team_size(rngs_[TPG_SEED]);
      } while (i == j);
      (team_to_mu)->MuProgramOrder(i, j);
   }
}

/******************************************************************************/
void TPG::TeamMutator_AddPrograms(team *team_to_mu) {
   double rd = real_dist_(rngs_[TPG_SEED]);
   if ((int)team_to_mu->size() < GetParam<int>("max_team_size") &&
       rd < GetParam<double>("pma")) {
      uniform_int_distribution<int> dis_programs(0, _L.size() - 1);
      uniform_int_distribution<int> dis_team_size(0, team_to_mu->size() - 1);
      int rand_p = dis_programs(rngs_[TPG_SEED]);
      int rand_ts = dis_team_size(rngs_[TPG_SEED]);
      program *p = _L[_Lids[rand_p]];
      team_to_mu->AddProgram(p, rand_ts);
   }
}

/******************************************************************************/
void TPG::TeamMutator_RemovePrograms(team *team_to_mu) {
   if (real_dist_(rngs_[TPG_SEED]) < GetParam<double>("pmd"))
      team_to_mu->RemoveRandomProgram(rngs_[TPG_SEED]);
}

/******************************************************************************/
team *TPG::CloneTeam(team *team_to_clone) {
   team *team_clone = new team(GetState("t_current"), state_["team_count"]++);
   for (auto m : team_to_clone->members_) {
      team_clone->AddProgram(m);
   }
   return team_clone;
}

/******************************************************************************/
program *TPG::CloneProgram(program *prog) {
   program *prog_clone = new RegisterMachine(
       *(dynamic_cast<RegisterMachine *>(prog)), params_, state_);
   if (prog_clone->action() >= 0)
      _teamMap[prog_clone->action()]->AddIncomingProgram(prog_clone->id_);
   return prog_clone;
}

/******************************************************************************/
void TPG::ProgramMutator_Instructions(program *prog_to_mu) {
   prog_to_mu->Mutate(params_, rngs_[TPG_SEED], _ops);
}

/******************************************************************************/
void TPG::MutateActionToTerminal(program *prog_to_mu, team *new_team) {
   // If program is already terminal (action < 0) and there are no discrete
   // actions there is nothing to change
   if (prog_to_mu->action() < 0 && GetParam<int>("n_discrete_action") == 0) {
      return;
   } else if (GetParam<int>("n_discrete_action") > 1) {
      uniform_int_distribution<int> dis(0,
                                        GetParam<int>("n_discrete_action") - 1);
      long new_discrete_action;
      do {
         // Discrete actions are negatives: -1 down to -n_discrete_action
         new_discrete_action = -1 - dis(rngs_[TPG_SEED]);
      } while (prog_to_mu->action() == new_discrete_action);
      // If changing from team pointer to atomic, update pointee's incoming
      if (prog_to_mu->action() >= 0) {
         _teamMap[prog_to_mu->action()]->removeIncomingProgram(prog_to_mu->id_);
      }
      prog_to_mu->muAction(new_discrete_action);
   }
}

/******************************************************************************/
void TPG::MutateActionToTeam(program *prog_to_mu, team *new_team,
                             int &n_new_teams) {
   // All programs remain terminal in the first generation
   if (GetState("t_current") == 1) {
      return;
   } else {
      uniform_int_distribution<int> disM(0, _teamMap.size() - 1);
      team *tm;
      int tries = 0;
      do {
         auto it = _teamMap.begin();
         advance(it, disM(rngs_[TPG_SEED]));
         tm = it->second;
      } while (tries++ < 20 &&
               (tm->gtime_ == GetState("t_current") || tm->clones_ > 0 ||
                prog_to_mu->action() == tm->id_));
      if (prog_to_mu->action() >= 0)
         _teamMap[prog_to_mu->action()]->removeIncomingProgram(prog_to_mu->id_);
      if (!tm->root()) {  // Already subsumed, don't clone
         prog_to_mu->muAction(tm->id_);
         tm->AddIncomingProgram(prog_to_mu->id_);
      } else {  // clone when subsumed
         team *sub = new team(GetState("t_current"), state_["team_count"]++);
         tm->clone(_phyloGraph, &sub);
         prog_to_mu->muAction(sub->id_);
         sub->AddIncomingProgram(prog_to_mu->id_);
         // TODO(skelly): put in PhyloGraph functions
         _phyloGraph[tm->id_].adj.push_back(sub->id_);
         _phyloGraph.insert(pair<long, phyloRecord>(sub->id_, phyloRecord()));
         _phyloGraph[sub->id_].gtime = GetState("t_current");
         _phyloGraph[sub->id_].root = false;
         AddTeam(sub);
         n_new_teams++;
      }
   }
}

/******************************************************************************/
void TPG::ProgramMutator_ActionPointer(program *prog_to_mu, team *new_team,
                                       int &n_new_teams) {
   if (real_dist_(rngs_[TPG_SEED]) < GetParam<double>("p_atomic")) {
      MutateActionToTerminal(prog_to_mu, new_team);
   } else {
      MutateActionToTeam(prog_to_mu, new_team, n_new_teams);
   }
}

/******************************************************************************/
void TPG::AddAncestorToPhylogeny(team *parent, team *new_team) {
   _phyloGraph[new_team->id_].ancestorIds.insert(parent->id_);
   new_team->addAncestorId(parent->id_);
   _phyloGraph[parent->id_].adj.push_back(new_team->id_);
}

/******************************************************************************/
void TPG::AddTeamToPhylogeny(team *new_team) {
   _phyloGraph.insert(pair<long, phyloRecord>(new_team->id_, phyloRecord()));
   _phyloGraph[new_team->id_].gtime = GetState("t_current");
   _phyloGraph[new_team->id_].root = new_team->root_;
}

/******************************************************************************/
team *TPG::TeamXover(vector<team *> &parents) {
   uniform_int_distribution<int> disP(0, parents.size() - 1);
   // parent teams
   team *pm1 = parents[disP(rngs_[TPG_SEED])];
   std::list<program *> p1programs = pm1->members_;
   auto p1liter = p1programs.begin();

   team *pm2 = parents[disP(rngs_[TPG_SEED])];
   std::list<program *> p2programs = pm2->members_;
   auto p2liter = p2programs.begin();

   team *child_team = new team(GetState("t_current"), state_["team_count"]++);

   while (p1liter != p1programs.end() || p2liter != p2programs.end()) {
      if (p1liter != p1programs.end()) {
         if ((*p1liter)->action() < 0 && child_team->n_atomic_ < 1) {
            child_team->AddProgram(*p1liter);
         } else if ((int)child_team->size() < GetParam<int>("max_team_size") &&
                    real_dist_(rngs_[TPG_SEED]) < GetParam<double>("pmx_p")) {
            child_team->AddProgram(*p1liter);
         }
      }
      if (p2liter != p2programs.end()) {
         if ((*p2liter)->action() < 0 && child_team->n_atomic_ < 1) {
            child_team->AddProgram(*p2liter);
         } else if ((int)child_team->size() < GetParam<int>("max_team_size") &&
                    real_dist_(rngs_[TPG_SEED]) < GetParam<double>("pmx_p")) {
            child_team->AddProgram(*p2liter);
         }
      }
      if (p1liter != p1programs.end()) p1liter++;
      if (p2liter != p2programs.end()) p2liter++;
   }

   if (child_team->n_atomic_ < 1) {
      die(__FILE__, __FUNCTION__, __LINE__,
          "Crossover must leave the fail-safe atomic program!");
   }
   AddTeamToPhylogeny(child_team);
   AddAncestorToPhylogeny(pm1, child_team);
   AddAncestorToPhylogeny(pm2, child_team);
   return child_team;
}

/******************************************************************************/
void TPG::GenerateNewTeams() {
   auto root_size_in = _Mroot.size();
   int new_teams_count = 0;
   auto task_power_set = PowerSet(GetState("n_task"));
   int n_new_teams_per_set =
       GetParam<int>("n_root_gen") / task_power_set.size();
   vector<team *> candidate_parent_teams;
   if (!GetParam<int>("parent_select_roots_only")) {
      candidate_parent_teams.resize(_M.size());
      std::copy(_M.begin(), _M.end(), candidate_parent_teams.begin());
   }
   for (auto &subset : task_power_set) {
      if (GetParam<int>("parent_select_roots_only")) {
         if (task_set_map_[vecToStrNoSpace(subset)].size() == 0) continue;
         candidate_parent_teams = task_set_map_[vecToStrNoSpace(subset)];
      }
      uniform_int_distribution<int> disP(0, candidate_parent_teams.size() - 1);
      for (int i = 0; i < n_new_teams_per_set; i++) {
         team *child_team;
         if (real_dist_(rngs_[TPG_SEED]) < GetParam<double>("pmx"))
            child_team = TeamXover(candidate_parent_teams);
         else {
            auto parent_team = candidate_parent_teams[disP(rngs_[TPG_SEED])];
            child_team = CloneTeam(parent_team);
            AddTeamToPhylogeny(child_team);
            AddAncestorToPhylogeny(parent_team, child_team);
         }
         // Mutate child team
         ApplyVariationOps(child_team, new_teams_count);
         AddTeam(child_team);
         new_teams_count++;
      }
   }
   oss << "genTms t " << GetState("t_current") << " Msz " << _M.size()
       << " Lsz " << _L.size() << " rSz " << _Mroot.size() << " rSzIn "
       << root_size_in << " mSz";
   for (int mem_t = 0; mem_t < memoryEigen::NUM_MEMORY_TYPES; mem_t++) {
      oss << " " << _Memory[mem_t].size();
   }
   oss << _Memory.size() << " eLSz "
       << _numEliteTeamsCurrent[GetState("phase")];
   oss << " nNTms " << new_teams_count << endl;
}

/******************************************************************************/
void TPG::ApplyVariationOps(team *team_to_modify, int &n_new_teams) {
   uniform_int_distribution<int> disL(0, _L.size() - 1);
   // Mutate team
   TeamMutator_RemovePrograms(team_to_modify);
   TeamMutator_AddPrograms(team_to_modify);
   TeamMutator_ProgramOrder(team_to_modify);
   // Mutate programs
   set<program *, programIdComp> new_team_programs =
       team_to_modify->CopyMembers();
   for (auto prog : new_team_programs) {
      // Probably modify one program
      if (real_dist_(rngs_[TPG_SEED]) < 1.0 / new_team_programs.size()) {
         team_to_modify->RemoveProgram(prog);
         program *prog_clone = CloneProgram(prog);
         ProgramMutator_Instructions(prog_clone);
         ProgramMutator_ActionPointer(prog_clone, team_to_modify, n_new_teams);
         team_to_modify->AddProgram(prog_clone);
         AddProgram(prog_clone);  // Add new program to program pop
      }
   }
}

/******************************************************************************/
// team* TPG::genTeamsInternal(long t,
//       mt19937 &rng,
//       set < team*, teamIdComp >  &internalReplacements, //new candidates
//       map < long, team* > &associatedRoots){
//
//    //get pool of internal teams
//    vector < team *> nonRootTeams;
//    for (auto it = _M.begin(); it != _M.end(); it++)
//       if (!(*it)->root())
//          nonRootTeams.push_back(*it);
//    uniform_int_distribution<int> disNonRootTeams(0,nonRootTeams.size()-1);
//    map <long, team*> parentPool;
//    int N1 = 10;
//    for (int i = 0; i < N1; i++){
//       team * tm = nonRootTeams[disNonRootTeams(rng)];
//       parentPool[tm->id_] = tm;
//       tm->resetPolicyRootIds();
//    }
//
//    //update associated root nodes for all internal teams
//    set <team*, teamIdComp> visitedTeams;
//    for (auto riter = _Mroot.begin(); riter != _Mroot.end(); riter++){
//       visitedTeams.clear();
//       long id = (*riter)->id_;
//       (*riter)->updatePolicyRoot(_teamMap, visitedTeams, id);
//    }
//
//    //count number of associated roots for each internal team
//    map <long, size_t> aRootSzMap;
//    for (auto ti = parentPool.begin(); ti != parentPool.end(); ti++){
//       //get associated roots
//       set < long > tmIds;
//       ti->second->policyRootIds(tmIds);
//       aRootSzMap[ti->second->id_] = tmIds.size();
//    }
//
//    //pick internal team with fewest associated roots
//    auto it = min_element(aRootSzMap.begin(), aRootSzMap.end(), [](const auto&
//    l, const auto& r) { return l.second < r.second; }); team * pm1 =
//    parentPool[it->first]; set < long > tmIds;
//    pm1->policyRootIds(tmIds);//this could just be done once
//
//    for (auto it = tmIds.begin(); it != tmIds.end(); it++){
//       associatedRoots[*it] = _teamMap[*it];
//       _teamMap[*it]->resetOutcomes(_VALIDATION_PHASE);
//    }
//
//    //parent2 for possible crossover
//    team *pm2 = _teamMap[_Mids[0]];
//    uniform_int_distribution<int> dis2(0,_Mids.size()-1);
//
//    //produce N children of pm1 (maybe pm2)
//    team *cm;
//    size_t numNewTeams = 0;
//    size_t N2 = 10;
//    while (numNewTeams < N2){
//       bool crossover = real_dist_(rng) < _pmx ? true : false;
//       if (crossover){
//          do { pm2 = _teamMap[_Mids[dis2(rng)]]; }
//          while (pm1->id_ == pm2->id_);
//       }
//       genTeams(t, pm1, pm2, crossover, &cm, numNewTeams);
//       if (crossover)
//          _phyloGraph[pm2->id_].adj.push_back(cm->id_);
//       _phyloGraph[pm1->id_].adj.push_back(cm->id_);
//       _phyloGraph.insert(pair<long, phyloRecord>(cm->id_, phyloRecord()));
//       _phyloGraph[cm->id_].gtime = GetState("t_current");
//       _phyloGraph[cm->id_].root = false;
//       addTeam(cm);
//       internalReplacements.insert(cm);
//       numNewTeams++;
//    }
//    return pm1;
// }

/******************************************************************************/
team *TPG::getBestTeam() {
   set<team *, teamFitnessLexicalCompare> teams;

   for (auto teiter = _Mroot.begin(); teiter != _Mroot.end(); teiter++)
      teams.insert(*teiter);

   return *teams.begin();
}

/******************************************************************************/
void TPG::UpdateTeamPhyloData(team *tm) {
   // MarkEffectiveCode(tm);
   if (tm->runTimeComplexityIns() == 0) {
      // TODO(skelly): clean out magic number - 2
      tm->updateComplexityRecord(_teamMap,
                                 GetParam<int>("n_point_aux_double") - 2);
   }
   _phyloGraph[tm->id_].fitness = tm->fit_;
   _phyloGraph[tm->id_].numActiveFeatures = tm->numActiveFeatures_;
   _phyloGraph[tm->id_].numActivePrograms = tm->numActivePrograms_;
   _phyloGraph[tm->id_].numActiveTeams = tm->numActiveTeams_;
   _phyloGraph[tm->id_].numEffectiveInstructions =
       tm->numEffectiveInstructions();
}

/******************************************************************************/
// Find the elite single-task program graphs
void TPG::FindSingleTaskFitnessRange(vector<TaskEnv *> &tasks,
                                     vector<vector<double>> &mins,
                                     vector<vector<double>> &maxs) {
   vector<team *> teamsRankedVec;
   for (int task = 0; task < GetState("n_task"); task++) {
      teamsRankedVec.clear();
      for (auto tm : _Mroot) {
         tm->elite(GetState("phase"), false);  // mark team as not elite
         if (tm->numOutcomes(GetState("phase"), task) >=
             tasks[task]->GetNumEval(GetState("phase"))) {
            tm->fit_ =
                tm->getQuickMean(task, GetState("fitMode"), GetState("phase"));
            teamsRankedVec.push_back(tm);
            if (GetState("phase") == _TEST_PHASE) {
               UpdateTeamPhyloData(tm);
            }
         } else {
            // mark elite to protect until eval in all tasks
            tm->elite(true);
         }
      }
      if (teamsRankedVec.size() > 0) {
         sort(teamsRankedVec.begin(), teamsRankedVec.end(),
              teamFitnessLexicalCompare());
         maxs[GetState("fitMode")][task] = (*(teamsRankedVec.begin()))->fit_;
         mins[GetState("fitMode")][task] = (*(teamsRankedVec.rbegin()))->fit_;
      }
   }
}

/******************************************************************************/
vector<team *> TPG::NormalizeScoresAndRankTeams(
    vector<TaskEnv *> &tasks, vector<int> &set,
    vector<vector<double>> &min_scores, vector<vector<double>> &max_scores) {
   vector<team *> vec;
   for (auto tm : _Mroot) {
      if (GetState("phase") == _TEST_PHASE &&
          tm->id_ != (_eliteTeamPS[vecToStrNoSpace(set)][GetParam<int>(
                          "fit_mode")][_VALIDATION_PHASE])
                         ->id_) {
         continue;
      }
      vector<double> normalizedScores;
      for (size_t task = 0; task < set.size(); task++) {
         if (tm->numOutcomes(GetState("phase"), set[task]) <
             tasks[set[task]]->GetNumEval(GetState("phase"))) {
            die(__FILE__, __FUNCTION__, __LINE__,
                "All root teams should have enough evaluations at this "
                "point.");
         }
         auto raw_mean_score = tm->getQuickMean(set[task], GetState("fitMode"),
                                                GetState("phase"));
         // guards for same min and max
         if (!isEqual(min_scores[GetState("fitMode")][set[task]],
                      max_scores[GetState("fitMode")][set[task]])) {
            normalizedScores.push_back(
                (raw_mean_score - min_scores[GetState("fitMode")][set[task]]) /
                (max_scores[GetState("fitMode")][set[task]] -
                 min_scores[GetState("fitMode")][set[task]]));
         } else {
            normalizedScores.push_back(
                raw_mean_score / max_scores[GetState("fitMode")][set[task]]);
         }
      }
      if (normalizedScores.size() != set.size()) {
         die(__FILE__, __FUNCTION__, __LINE__,
             "This team should have a score for all tasks.");
      }
      tm->fit_ = *min_element(normalizedScores.begin(), normalizedScores.end());
      // TODO(skelly): debug, test, and cleanup complexity record
      // GetParam<int>("n_point_aux_double") - 2 refers to
      // decision_instructions
      tm->updateComplexityRecord(_teamMap,
                                 GetParam<int>("n_point_aux_double") - 1);
      vec.push_back(tm);
   }
   return vec;
}

/******************************************************************************/
void TPG::FindMultiTaskElites(vector<TaskEnv *> &tasks,
                              vector<vector<double>> &min_scores,
                              vector<vector<double>> &max_scores) {
   auto PS = PowerSet(GetState("n_task"));
   size_t n_elite_per_task = GetParam<int>("n_root") / PS.size();
   for (auto &set : PS) {
      if (GetState("phase") == _TRAIN_PHASE)
         task_set_map_[vecToStrNoSpace(set)]
             .clear();  // TODO(skelly): check this
      auto teams_normed_scores =
          NormalizeScoresAndRankTeams(tasks, set, min_scores, max_scores);

      sort(teams_normed_scores.begin(), teams_normed_scores.end(),
           teamFitnessLexicalCompare());
      // sort(teams_normed_scores.begin(), teams_normed_scores.end(),
      // teamFitComplexLexCompare());
      size_t elite_count = 0;
      for (auto tm : teams_normed_scores) {
         if (!tm->elite(GetState("phase"))) {
            elite_count++;
            _numEliteTeamsCurrent[GetState("phase")]++;
            tm->elite(GetState("phase"), true);
            if (GetState("phase") == _TRAIN_PHASE) {
               tm->fitnessBin(GetState("t_current"), vecToStrNoSpace(set));
               _phyloGraph[tm->id_].fitnessBin = tm->fitnessBin();
               _phyloGraph[tm->id_].fitness = tm->fit_;

               _phyloGraph[tm->id_].taskFitnesses.clear();
               for (int task = 0; task < GetState("n_task"); task++) {
                  _phyloGraph[tm->id_].taskFitnesses.push_back(tm->getQuickMean(
                      task, GetState("fitMode"), GetState("phase")));
               }
            }
            if (GetState("phase") == _TRAIN_PHASE)
               task_set_map_[vecToStrNoSpace(set)].push_back(tm);
         }
         if (elite_count >= n_elite_per_task) break;
      }
      // Always keep track of single elite team for each task set
      _eliteTeamPS[vecToStrNoSpace(set)][GetState("fitMode")]
                  [GetState("phase")] = *(teams_normed_scores.begin());
   }
}

/******************************************************************************/
void TPG::SetEliteTeams(vector<TaskEnv *> &tasks) {
   vector<team *> teams_normed_scores;
   _numEliteTeamsCurrent[GetState("phase")] = 0;
   // min/max scores for normalization, overcomplicated data structure?
   vector<vector<double>> min_scores, max_scores;
   min_scores.resize(GetParam<int>("n_fit_mode"));
   max_scores.resize(GetParam<int>("n_fit_mode"));
   min_scores[GetState("fitMode")].resize(GetState("n_task"));
   max_scores[GetState("fitMode")].resize(GetState("n_task"));

   FindSingleTaskFitnessRange(tasks, min_scores, max_scores);
   FindMultiTaskElites(tasks, min_scores, max_scores);

   auto PS = PowerSet(GetState("n_task"));
   for (auto &set : PS) {
      auto elite_id = _eliteTeamPS[vecToStrNoSpace(set)][GetState("fitMode")]
                                  [GetState("phase")]
                                      ->id_;
      if (set.size() == 1 &&
          haveEliteTeam(vecToStrNoSpace(set), GetState("fitMode"),
                        GetState("phase"))) {
         oss << "setElTmsST eLSz " << _numEliteTeamsCurrent[GetState("phase")]
             << " ss " << vecToStrNoSpace(set) << " fm " << GetState("fitMode")
             << " minThr "
             << _eliteTeamPS[vecToStrNoSpace(set)][GetState("fitMode")]
                            [GetState("phase")]
                                ->fit_
             << " ";
         printTeamInfo(GetState("t_current"), GetState("phase"), false,
                       elite_id);

         if (GetParam<int>("track_experiments") &&
             GetState("t_current") % GetParam<int>("track_mod") == 0) {
            trackTeamInfo(GetState("t_current"), GetState("phase"), false,
                          elite_id);
         }
      }
      if (set.size() == (size_t)GetState("n_task") &&
          haveEliteTeam(vecToStrNoSpace(set), GetState("fitMode"),
                        GetState("phase"))) {
         oss << "setElTmsMTA eLSz " << _numEliteTeamsCurrent[GetState("phase")]
             << " ss " << vecToStrNoSpace(set) << " fm " << GetState("fitMode")
             << " minThr "
             << _eliteTeamPS[vecToStrNoSpace(set)][GetState("fitMode")]
                            [GetState("phase")]
                                ->fit_
             << " ";
         printTeamInfo(GetState("t_current"), GetState("phase"), false,
                       elite_id);

         if (GetParam<int>("track_experiments") &&
             GetState("t_current") % GetParam<int>("track_mod") == 0) {
            trackTeamInfo(GetState("t_current"), GetState("phase"), false,
                          elite_id);
         }

         // Keep track of elite team history and only save checkpoints
         // when we have a new test champion for this phase
         if (HaveParam("save_champ_checkpoints") &&
             GetState("phase") == GetParam<int>("save_champ_checkpoints") &&
             elite_team_id_history_.find(elite_id) ==
                 elite_team_id_history_.end()) {
            elite_team_id_history_.insert(elite_id);
            WriteCheckpoint(GetState("t_current"), true);
         }
      }
   }
}

/******************************************************************************/
// void TPG::internalReplacementPareto(
//       int phase,
//       int fitMode,
//       int complexityAuxDouble,
//       team * tm,
//       map < long, team* > &associatedRoots,
//       set < team*, teamIdComp > &internalReplacements,
//       mt19937 &rng){
//    (void)rng;
//    vector <double> min_scores, max_scores;
//    min_scores.resize(GetState("n_task"));
//    max_scores.resize(GetState("n_task"));
//
//    vector <team *> teamsRankedVec;
//    teamsRankedVec.reserve(internalReplacements.size() *
//    GetState("n_task"));
//
//    vector <double> internalTeamMeanReward;
//    internalTeamMeanReward.reserve(associatedRoots.size());
//    internalTeamMeanReward.resize(associatedRoots.size());
//    vector <double> internalTeamMeanComplexity;
//    internalTeamMeanComplexity.reserve(associatedRoots.size());
//    internalTeamMeanComplexity.resize(associatedRoots.size());
//    for (size_t o = 0; o < GetState("n_task"); o++){
//       teamsRankedVec.clear();
//       for(auto teiterInternal = internalReplacements.begin(); teiterInternal
//       != internalReplacements.end(); teiterInternal++){
//          int r = 0;
//          for (auto teiterRoots = associatedRoots.begin(); teiterRoots !=
//          associatedRoots.end(); teiterRoots++){
//             internalTeamMeanReward[r] =
//             teiterRoots->second->getMeanOutcome(phase, o, fitMode,
//             POINT_AUX_INT_internalTestNodeId, (*teiterInternal)->id_, false,
//             false); internalTeamMeanComplexity[r++] =
//             teiterRoots->second->getMeanOutcome(phase, o,
//             complexityAuxDouble, POINT_AUX_INT_internalTestNodeId,
//             (*teiterInternal)->id_, false, false);
//          }
//          (*teiterInternal)->setQuickMean(o, fitMode, phase,
//          vecMedian(internalTeamMeanReward));
//          (*teiterInternal)->fit((*teiterInternal)->getQuickMean(o, fitMode,
//          phase));
//          (*teiterInternal)->runTimeComplexityIns(vecMedian(internalTeamMeanComplexity));
//          teamsRankedVec.push_back(*teiterInternal);
//       }
//       if (teamsRankedVec.size() > 0){
//          sort(teamsRankedVec.begin(), teamsRankedVec.end(),
//          teamFitnessCompare()); max_scores[o] =
//          (*(teamsRankedVec.begin()))->fit(); min_scores[o] =
//          (*(teamsRankedVec.rbegin()))->fit();
//       }
//    }
//
//    //re-map fitness score to minimum normalized score over all objectives
//    teamsRankedVec.clear();
//    vector <double> normalizedScores;
//    double rawMeanScore;
//    for(auto teiterInternal = internalReplacements.begin(); teiterInternal !=
//    internalReplacements.end(); teiterInternal++){
//       normalizedScores.clear();
//       for (size_t o = 0; o < GetState("n_task"); o++){
//          rawMeanScore = (*teiterInternal)->getQuickMean(o, fitMode, phase);
//          if (!isEqual(min_scores[o], max_scores[0]))
//             normalizedScores.push_back((rawMeanScore -
//             min_scores[o])/(max_scores[o] - min_scores[o]));
//          else
//             normalizedScores.push_back(rawMeanScore / min_scores[o]);
//          //watch out for nan here in case of zero scores...
//       }
//       (*teiterInternal)->fit(*min_element(normalizedScores.begin(),
//       normalizedScores.end())); SetL.push_back(*teiterInternal);
//    }
//
//    ////find pareto front wrt fitness & complexity
//    //vector < team* > paretoFrontTeams;
//    //double minComplexity = numeric_limits<double>::max();
//    //sort(teamsRankedVec.begin(), teamsRankedVec.end(),
//    teamFitnessLexicalCompare());
//    //for (auto itA = teamsRankedVec.begin(); itA != teamsRankedVec.end();
//    itA++){
//    //addFront:
//    //   minComplexity = min(minComplexity, (*itA)->runTimeComplexityIns());
//    //   paretoFrontTeams.push_back(*itA);
//    //   auto itB = next(itA, 1);
//    //   while (itB != teamsRankedVec.end()){
//    //      if ((*itB)->runTimeComplexityIns() < minComplexity){
//    //         itA = itB;
//    //         goto addFront;
//    //      }
//    //      itB++;
//    //   }
//    //   break;
//    //}
//    ////DEBUG PARETO BY PLOTTING
//    //cout << " pFront size " << paretoFrontTeams.size() << endl;
//    //for (auto it = paretoFrontTeams.begin(); it != paretoFrontTeams.end();
//    it ++)
//    //   cout << "pfront t " << GetState("t_current") << " " << (*it)->id_ <<
//    " " << (*it)->fit() << " " << -1*(*it)->runTimeComplexityIns() << endl;
//
//    //uniform_int_distribution<int> disP(0, paretoFrontTeams.size() - 1);
//    //team* paretoWinner = paretoFrontTeams[disP(rng)];
//
//    sort(teamsRankedVec.begin(), teamsRankedVec.end(),
//    teamFitComplexLexCompare()); team* paretoWinner =
//    *(teamsRankedVec.begin());
//
//    //if (isEqual(paretoWinner->fit(), tm->fit()))
//    //   paretoWinner = tm;
//
//    //update incoming program pointers
//    set < long > incomingProgramIds;
//    tm->incomingPrograms(incomingProgramIds);
//    for (auto piditer = incomingProgramIds.begin(); piditer !=
//    incomingProgramIds.end(); piditer++){
//       tm->removeIncomingProgram(*piditer);
//       _L[*piditer]->action(paretoWinner->id_);
//       paretoWinner->addIncomingProgram(*piditer);
//    }
//
//    //delete the unused candidates
//    internalReplacements.erase(paretoWinner);
//    deque < program* > programsWithNoRefs;
//    for (auto teiter = internalReplacements.begin(); teiter !=
//    internalReplacements.end(); teiter++){
//       _phyloGraph[(*teiter)->id_].dtime = GetState("t_current");
//       (*teiter)->cleanup(_teamMap, programsWithNoRefs);
//       _teamMap.erase((*teiter)->id_);
//       removeTeam(*teiter, true);//_M.erase(*teiter);
//       _Mroot.erase(*teiter);
//       delete *teiter;
//    }
//    cleanupProgramsWithNoRefs(GetState("t_current"), programsWithNoRefs,
//    true);
//
//    //update score wrt paretoWinner
//    for (auto teiterRoots = associatedRoots.begin(); teiterRoots !=
//    associatedRoots.end(); teiterRoots++)
//       teiterRoots->second->swapOutcomePhase(phase, _TRAIN_PHASE,
//       POINT_AUX_INT_internalTestNodeId, paretoWinner->id_);
// }

/******************************************************************************/
// Helper struct for distance comparisons
struct distanceInstance {
   double distance;
   bool fromArchive;
   distanceInstance(double d, bool a) : distance(d), fromArchive(a) {}
};

/******************************************************************************/
bool compareByDistance(const distanceInstance &a, const distanceInstance &b) {
   return a.distance < b.distance;
}

/******************************************************************************/
void TPG::InitTeams() {
   int max_discrete_action = GetParam<int>("n_discrete_action") > 0
                                 ? GetParam<int>("n_discrete_action") - 1
                                 : 0;
   uniform_int_distribution<int> dis_actions(0, max_discrete_action);
   int initial_team_size = 2;
   for (int t = 0; t < GetParam<int>("n_root"); t++) {
      auto new_team = new team(GetState("t_current"), state_["team_count"]++);
      for (int p = 0; p < initial_team_size; p++) {
         // Discrete atomic actions are negatives -1 to -numAtomicActions()
         long discrete_action = -1 - dis_actions(rngs_[TPG_SEED]);
         auto new_prog = new RegisterMachine(discrete_action, params_, state_,
                                             rngs_[TPG_SEED], _ops);
         new_team->AddProgram(new_prog);
         AddProgram(new_prog);  // add program to program population
      }
      AddTeam(new_team);  // add team to team population
      _phyloGraph.insert(pair<long, phyloRecord>(new_team->id_, phyloRecord()));
      _phyloGraph[new_team->id_].gtime = 0;
   }
   // Fill teams from learner population
   uniform_int_distribution<int> dis_team_size(
       2, GetParam<int>("max_initial_team_size"));
   uniform_int_distribution<int> dis_programs(0, _L.size() - 1);
   for (auto tm : _M) {
      auto team_size = dis_team_size(rngs_[TPG_SEED]);
      while (tm->size() < team_size)
         tm->AddProgram(_L[dis_programs(rngs_[TPG_SEED])]);
   }
   oss << "InitTms Msz " << _M.size() << " Lsz " << _L.size() << " rSz "
       << _Mroot.size() << " mSz";
   for (int mem_t = 0; mem_t < memoryEigen::NUM_MEMORY_TYPES; mem_t++) {
      oss << " " << _Memory[mem_t].size();
   }
   oss << " eLSz " << _numEliteTeamsCurrent[GetState("phase")] << endl;
}

/******************************************************************************/
// Certain parameters must be processed here
void TPG::ProcessParams() {
   Seed(TPG_SEED, GetParam<int>("seed_tpg"));
   Seed(AUX_SEED, GetParam<int>("seed_aux"));
   // Replaying will require starting from checkpoint
   if (GetParam<int>("replay")) params_["start_from_checkpoint"] = 1;
   // Set the starting and current generation (t_start and t_current)
   if (GetParam<int>("start_from_checkpoint")) {
      state_["t_start"] = GetParam<int>("checkpoint_in_t") + 1;
   } else {
      state_["t_start"] = 0;
   }
   state_["t_current"] = GetState("t_start");
}

/******************************************************************************/
// Parameters are set in the parameters.txt file.
// TPG can also process command line parameters in the form: <name>=<value>
// <name> must be a parameter with a default value in parameters.txt
// Default values are overwritten by command line parameters
void TPG::SetParams(int argc, char **argv) {
   // First read parameters file
   ReadParameters("parameters.txt", params_);
   // Parse command line parameters
   if (argc > 1) {
      for (int i = 1; i < argc; ++i) {
         std::string arg = argv[i];
         size_t pos = arg.find('=');
         if (pos != std::string::npos) {
            std::string key = arg.substr(0, pos);
            std::string val = arg.substr(pos + 1);
            if (HaveParam(key)) {
               if (val.find('.') != std::string::npos) {
                  params_[key] = stringToDouble(val);
               } else {
                  params_[key] = stringToInt(val);
               }
            } else {
               std::string err_message =
                   "Unreconised command line parameter:" + key +
                   ". Command line parameters must have default values in "
                   "parameters.txt";
               die(__FILE__, __FUNCTION__, __LINE__, err_message.c_str());
            }
         }
      }
   }
   ProcessParams();
}

/******************************************************************************/
void TPG::policyFeatures(int hostId, set<long> &features, bool active) {
   features.clear();
   set<team *, teamIdComp> visitedTeams;
   if (hostId == -1) {
      for (auto it = _Mroot.begin(); it != _Mroot.end(); it++) {
         visitedTeams.clear();
         (*it)->policyFeatures(_teamMap, visitedTeams, features, active);
      }
   } else
      _teamMap[hostId]->policyFeatures(_teamMap, visitedTeams, features,
                                       active);
}

// /******************************************************************************/
// // Print graph defined by <rootTeam> in DOT format for GraphViz
// void TPG::printGraphDot(
//     team *rootTeam, size_t frame, int episode, int step, size_t depth,
//     vector<program *> allPrograms, vector<program *> winningPrograms,
//     vector<set<long>> decisionFeatures,
//     vector<set<memoryEigen *, memoryEigenIdComp>> decisionMemories,
//     vector<team *> teamPath, bool drawPath,
//     set<team *, teamIdComp> visitedTeamsAllTasks) {
//   // unused arguments
//   (void)decisionFeatures;
//   (void)decisionMemories;
//   (void)allPrograms;

//   // just use winning programs up to a specific graph depth
//   // vector<program*> winningProgramsDepth(winningPrograms.begin(),
//   // winningPrograms.begin()+depth);

//   vector<program *> winningProgramsDepth(winningPrograms.begin(),
//                                          winningPrograms.end());

//   double nodeWidth = 2.0;
//   double edgeWidth_1 = 1;  // 5;
//   double edgeWidth_2 = 30;
//   double arrowSize_1 = 1;  // 0.1;
//   double arrowSize_2 = 1;  // 0.2;

//   char outputFilename[80];
//   ofstream ofs;

//   set<team *, teamIdComp> teams;
//   set<program *, programIdComp> programs;
//   set<memoryEigen *, memoryEigenIdComp> memories;

//   //(void)visitedTeamsAllTasks;
//   for (auto it = visitedTeamsAllTasks.begin(); it !=
//   visitedTeamsAllTasks.end();
//        it++) {
//     set<program *, programIdComp> p = (*it)->CopyMembers();
//     programs.insert(p.begin(), p.end());
//   }
//   for (auto it = programs.begin(); it != programs.end(); it++) {
//     for (int mem_t = 0; mem_t < memoryEigen::NUM_MEMORY_TYPES; mem_t++) {
//       memories.insert((*it)->MemGet(mem_t));
//     }
//   }

//   sprintf(outputFilename, "replay/graphs/gv_%d_%05d_%03d_%05d_%05d%s",
//           (int)rootTeam->id_, (int)frame, episode, step, (int)depth, ".dot");
//   ofs.open(outputFilename, ios::out);
//   if (!ofs) die(__FILE__, __FUNCTION__, __LINE__, "Can't open file.");

//   ofs << "digraph G {" << endl;
//   ofs << "ratio=1" << endl;
//   ofs << "root=t_" << rootTeam->id_ << endl;

//   ////atomic actions
//   // for(auto leiter = programs.begin(); leiter != programs.end(); leiter++)
//   //    if ((*leiter)->action() < 0)
//   //       ofs << " a_" << ((*leiter)->action()*-1)-1 << "_" <<
//   (*leiter)->id_
//   //       << " [shape=point, label=\"\", regular=1, width=0.1]" << endl;

//   // programs
//   for (auto leiter = programs.begin(); leiter != programs.end(); leiter++) {
//     if (step > 0 &&
//         find(winningProgramsDepth.begin(), winningProgramsDepth.end(),
//              *leiter) != winningProgramsDepth.end()) {
//       // ofs << " p_" << (*leiter)->id_ << " [shape=box, style=filled,
//       // color=black, label=\"\", fontsize=200, regular=1, width=" <<
//       nodeWidth
//       // << "]" << endl;
//       ofs << " p_" << (*leiter)->id_
//           << " [shape=box, style=filled, color=green, label=\"\", "
//              "fontsize=200, regular=1, width="
//           << nodeWidth << "]" << endl;
//     } else if (step > 0 && find(allPrograms.begin(), allPrograms.end(),
//                                 *leiter) != allPrograms.end())
//       ofs << " p_" << (*leiter)->id_
//           << " [shape=box, style=filled, color=black, label=\"\", "
//              "fontsize=200, regular=1, width="
//           << nodeWidth << "]" << endl;
//     else if (teamPath.size() > 0)
//       ofs << " p_" << (*leiter)->id_
//           << " [shape=box, style=filled, color=grey90, label=\"\", "
//              "fontsize=200, regular=1, width="
//           << nodeWidth << "]" << endl;
//     else
//       ofs << " p_" << (*leiter)->id_
//           << " [shape=box, style=filled, color=grey70, label=\"\", "
//              "fontsize=200, regular=1, width="
//           << nodeWidth << "]" << endl;
//   }

//   // teams
//   int label = 0;
//   // for(auto teiter = teams.begin(); teiter != teams.end(); teiter++)
//   for (auto teiter = visitedTeamsAllTasks.begin();
//        teiter != visitedTeamsAllTasks.end(); teiter++)
//     if ((*teiter)->id_ == rootTeam->id_ ||
//         (step > 0 &&
//          find(teamPath.begin(), teamPath.end(), *teiter) != teamPath.end()))
//       ofs << " t_" << (*teiter)->id_
//           << " [shape=circle, style=filled, fillcolor=black, label=\"t"
//           << label++ << "\", fontsize=84, regular=1, width=" << nodeWidth * 2
//           << "]" << endl;
//     else
//       // ofs << " t_" << (*teiter)->id_ << " [shape=circle, style=filled,
//       // fillcolor=grey90, label=\"\", fontsize=84, regular=1, width=" <<
//       // nodeWidth*2 << "]" << endl;
//       ofs << " t_" << (*teiter)->id_
//           << " [shape=circle, style=filled, fillcolor=deepskyblue,
//           label=\"\", "
//              "fontsize=84, regular=1, width="
//           << nodeWidth * 2 << "]" << endl;

//   ////memoryEigen registers
//   // for(auto meiter = memories.begin(); meiter != memories.end(); meiter++)
//   //    ofs << " m_" << (*meiter)->id_ << " [shape=invhouse, style=filled,
//   //    fillcolor=grey, label=\"\", regular=1, width=" << nodeWidth << "]" <<
//   //    endl;

//   // program -> team edges
//   for (auto leiter = programs.begin(); leiter != programs.end(); leiter++) {
//     if ((*leiter)->action() >= 0 &&
//         find(visitedTeamsAllTasks.begin(), visitedTeamsAllTasks.end(),
//              _teamMap[(*leiter)->action()]) != visitedTeamsAllTasks.end()) {
//       double w = find(winningProgramsDepth.begin(),
//       winningProgramsDepth.end(),
//                       *leiter) == winningProgramsDepth.end() ||
//                          !drawPath
//                      ? edgeWidth_1
//                      : edgeWidth_2;
//       if (teamPath.size() < 1) w = 5;
//       double as = find(winningProgramsDepth.begin(),
//       winningProgramsDepth.end(),
//                        *leiter) == winningProgramsDepth.end() ||
//                           !drawPath
//                       ? arrowSize_1
//                       : arrowSize_2;
//       string col =
//           find(winningProgramsDepth.begin(), winningProgramsDepth.end(),
//                *leiter) == winningProgramsDepth.end()
//               ? "black"
//               : "green";
//       ofs << " p_" << (*leiter)->id_ << "->"
//           << "t_" << (*leiter)->action() << " [arrowsize=" << as
//           << ", penwidth=" << w << " color=" << col.c_str() << "];" << endl;
//     }
//   }

//   ////program -> memoryEigen edges
//   // for(auto leiter = programs.begin(); leiter != programs.end(); leiter++){
//   //    double w = find(winningProgramsDepth.begin(),
//   //    winningProgramsDepth.end(), *leiter) == winningProgramsDepth.end() ?
//   //    edgeWidth_1 : edgeWidth_2; double as =
//   //    find(winningProgramsDepth.begin(), winningProgramsDepth.end(),
//   *leiter)
//   //    == winningProgramsDepth.end() ? arrowSize_1 : arrowSize_2; ofs << "
//   p_"
//   //    << (*leiter)->id_ << "->" << "m_"<< (*leiter)->MemGet()->id_;
//   //       ofs << " [dir=both, arrowsize=" << as << ", penwidth=" << w <<
//   "];"
//   //       << endl;
//   // }

//   // team -> program edges
//   // for(auto teiter = teams.begin(); teiter != teams.end(); teiter++){
//   for (auto teiter = visitedTeamsAllTasks.begin();
//        teiter != visitedTeamsAllTasks.end(); teiter++) {
//     list<program *> mem;
//     (*teiter)->members(&mem);
//     for (auto leiter = mem.begin(); leiter != mem.end(); leiter++) {
//       double w =
//           find(teamPath.begin(), teamPath.end(), *teiter) == teamPath.end()
//           ||
//                   !drawPath
//               ? edgeWidth_1
//               : edgeWidth_2;
//       if (teamPath.size() < 1) w = 5;
//       double as =
//           find(teamPath.begin(), teamPath.end(), *teiter) == teamPath.end()
//           ||
//                   !drawPath
//               ? arrowSize_1
//               : arrowSize_2;
//       string col =
//           find(winningProgramsDepth.begin(), winningProgramsDepth.end(),
//                *leiter) == winningProgramsDepth.end()
//               ? "black"
//               : "green";
//       ofs << " t_" << (*teiter)->id_ << "->p_" << (*leiter)->id_
//           << " [arrowsize=" << as << ", penwidth=" << w
//           << " color=" << col.c_str() << "];" << endl;
//     }
//   }
//   ofs << "}" << endl;
//   ofs.close();
// }

// /******************************************************************************/
// void TPG::printGraphDotGPEM(long rootTeamId, map<long, string> &teamColMap,
//                             set<team *, teamIdComp> &visitedTeamsAllTasks,
//                             vector<map<long, double>> &teamUseMapPerTask) {
//   team *rootTeam = _teamMap[rootTeamId];

//   (void)teamColMap;
//   vector<string> taskCol;
//   taskCol.push_back("#7fc97f");
//   taskCol.push_back("#beaed4");
//   taskCol.push_back("#fdc086");
//   taskCol.push_back("#ffff99");
//   taskCol.push_back("#386cb0");
//   taskCol.push_back("#f0027f");
//   map<long, string> nodeLabMap;

//   nodeLabMap[1594815] = "1";  //   29000"
//   nodeLabMap[624659] = "2";   //  149"
//   nodeLabMap[1302870] = "3";  //   28373"
//   nodeLabMap[623346] = "4";   //  580"
//   nodeLabMap[493990] = "5";   //  149"
//   nodeLabMap[836830] = "6";   //  28019"
//   nodeLabMap[548151] = "7";   //  832"
//   nodeLabMap[126871] = "8";   //  1373"
//   nodeLabMap[425177] = "9";   //  774"
//   nodeLabMap[602173] = "10";  //   1826"
//   nodeLabMap[42314] = "11";   //  9"
//   nodeLabMap[26879] = "12";   //  7"
//   nodeLabMap[200127] = "13";  //   4"
//   nodeLabMap[470578] = "14";  //   1826"
//   nodeLabMap[5964] = "15";    // 7"
//   nodeLabMap[23266] = "16";   //  2"
//   nodeLabMap[226807] = "17";  //   394"
//   nodeLabMap[180005] = "18";  //   953"

//   double nodeWidth = 2.0;
//   double arrowSize_1 = 3;  // 0.1;

//   char outputFilename[80];
//   ofstream ofs;

//   set<team *, teamIdComp> teams;
//   set<program *, programIdComp> programs;
//   set<memoryEigen *, memoryEigenIdComp> memories;

//   for (auto it = visitedTeamsAllTasks.begin(); it !=
//   visitedTeamsAllTasks.end();
//        it++) {
//     set<program *, programIdComp> p = (*it)->CopyMembers();  // no need o
//     copy programs.insert(p.begin(), p.end());
//   }
//   for (auto it = programs.begin(); it != programs.end(); it++) {
//     for (int mem_t = 0; mem_t < memoryEigen::NUM_MEMORY_TYPES; mem_t++) {
//       memories.insert((*it)->MemGet(mem_t));
//     }
//   }

//   sprintf(outputFilename,
//           "replay/gv_taskDecomposition_%d_%05d_%03d_%05d_%05d%s",
//           (int)rootTeam->id_, 0, 0, 0, 0, ".dot");
//   ofs.open(outputFilename, ios::out);
//   if (!ofs) die(__FILE__, __FUNCTION__, __LINE__, "Can't open file.");

//   ofs << "strict digraph G {" << endl;
//   ofs << "ratio=0.7" << endl;
//   ofs << "root=t_" << rootTeam->id_ << endl;

//   ////programs
//   // for(auto leiter = programs.begin(); leiter != programs.end(); leiter++){
//   //    ofs << " p_" << (*leiter)->id_ << " [shape=box, style=filled,
//   //    color=grey70, label=\"\", fontsize=200, regular=1, width=" <<
//   nodeWidth
//   //    << "]" << endl;
//   // }

//   // teams
//   for (auto teiter = visitedTeamsAllTasks.begin();
//        teiter != visitedTeamsAllTasks.end(); teiter++) {
//     string col = "";
//     // if (teamColMap.find((*teiter)->id_) != teamColMap.end())
//     //    col = teamColMap[(*teiter)->id_];
//     // else
//     //    col = "white";

//     ofs << " t_" << (*teiter)->id_
//         << " [shape=circle, style=wedged, fillcolor=\"";
//     double sumUse = 0;
//     for (int tsk = 0; tsk < 6; tsk++)
//       sumUse += teamUseMapPerTask[tsk][(*teiter)->id_];
//     for (int tsk = 0; tsk < 6; tsk++) {
//       ofs << taskCol[tsk] << ";"
//           << (teamUseMapPerTask[tsk].find((*teiter)->id_) !=
//                       teamUseMapPerTask[tsk].end()
//                   ? teamUseMapPerTask[tsk][(*teiter)->id_] / sumUse
//                   : 0);
//       if (tsk < 5) ofs << ":";
//     }
//     // ofs << ":" << taskCol[1] << ";"<<
//     // teamUseMapPerTask[0].find((*teiter)->id_) !=
//     teamUseMapPerTask[0].end()
//     // ? teamUseMapPerTask[0][(*teiter)->id_]/6: "0"; ofs << ":" <<
//     taskCol[2]
//     // << ";"<< teamUseMapPerTask[0].find((*teiter)->id_) !=
//     // teamUseMapPerTask[0].end() ? teamUseMapPerTask[0][(*teiter)->id_]/6:
//     // "0"; ofs << ":" << taskCol[3] << ";"<<
//     // teamUseMapPerTask[0].find((*teiter)->id_) !=
//     teamUseMapPerTask[0].end()
//     // ? teamUseMapPerTask[0][(*teiter)->id_]/6: "0"; ofs << ":" <<
//     taskCol[4]
//     // << ";"<< teamUseMapPerTask[0].find((*teiter)->id_) !=
//     // teamUseMapPerTask[0].end() ? teamUseMapPerTask[0][(*teiter)->id_]/6:
//     // "0"; ofs << ":" << taskCol[5] << ";"<<
//     // teamUseMapPerTask[0].find((*teiter)->id_) !=
//     teamUseMapPerTask[0].end()
//     // ? teamUseMapPerTask[0][(*teiter)->id_]/6: "0";
//     ofs << "\"";

//     ofs << ", label=\""
//         << (nodeLabMap.find((*teiter)->id_) == nodeLabMap.end()
//                 ? ""
//                 : nodeLabMap[(*teiter)->id_])
//         << "\", fontsize=84, regular=1, width=" << nodeWidth * 2
//         << ",penwidth=0]" << endl;
//     // ofs << " t_" << (*teiter)->id_ << " [shape=circle, style=wedged,
//     // fillcolor=\"" << col << "\", label=\"\", fontsize=84, regular=1,
//     width="
//     // << nodeWidth*2 << "]" << endl;
//   }

//   ////program -> team edges
//   // for(auto leiter = programs.begin(); leiter != programs.end(); leiter++){
//   //    if ((*leiter)->action() >= 0 && find(visitedTeamsAllTasks.begin(),
//   //    visitedTeamsAllTasks.end(), _teamMap[(*leiter)->action()]) !=
//   //    visitedTeamsAllTasks.end()){
//   //       ofs << " p_" << (*leiter)->id_ << "->" << "t_"<<
//   (*leiter)->action()
//   //       << " [arrowsize=" << arrowSize_1 << ", penwidth=" << "1" << "
//   color="
//   //       << "black" << "];" << endl;
//   //    }
//   // }

//   ////team -> program edges
//   // for(auto teiter = visitedTeamsAllTasks.begin(); teiter !=
//   // visitedTeamsAllTasks.end(); teiter++){
//   //    list < program * > mem;
//   //    (*teiter)->members(&mem);
//   //    for(auto leiter = mem.begin(); leiter != mem.end(); leiter++){
//   //       ofs << " t_" << (*teiter)->id_ << "->p_" << (*leiter)->id_ << "
//   //       [arrowsize=" << arrowSize_1  << ", penwidth=" << "1" << " color="
//   <<
//   //       "black" << "];" << endl;
//   //    }
//   // }

//   // team -> team edges
//   for (auto teiter = visitedTeamsAllTasks.begin();
//        teiter != visitedTeamsAllTasks.end(); teiter++) {
//     list<program *> mem;
//     (*teiter)->members(&mem);
//     for (auto leiter = mem.begin(); leiter != mem.end(); leiter++) {
//       // ofs << " t_" << (*teiter)->id_ << "->p_" << (*leiter)->id_ << "
//       // [arrowsize=" << arrowSize_1  << ", penwidth=" << "1" << " color=" <<
//       // "black" << "];" << endl;
//       if ((*leiter)->action() >= 0 &&
//           find(visitedTeamsAllTasks.begin(), visitedTeamsAllTasks.end(),
//                _teamMap[(*leiter)->action()]) != visitedTeamsAllTasks.end())
//                {
//         ofs << " t_" << (*teiter)->id_ << "->t_" << (*leiter)->action()
//             << " [arrowsize=" << arrowSize_1 << ", penwidth="
//             << "1"
//             << " color="
//             << "black"
//             << "];" << endl;
//       }
//     }
//   }

//   ////legend
//   // ofs << "subgraph {" << endl;
//   // ofs << "ratio=1" << endl;
//   // ofs << "rank=sink" << endl;
//   // ofs << "node [shape=plaintext]" << endl;
//   // ofs << "legend [colorscheme=set18," << endl;
//   // ofs << "label=<" << endl;
//   //
//   // ofs << "<table border=\"0\" cellborder=\"1\" cellspacing=\"0\">" <<
//   endl;
//   // ofs << "<tr><td bgcolor=\"" << taskCol[0] << "\">" << "CartPole" <<
//   // "</td></tr>" << endl; ofs << "<tr><td bgcolor=\"" << taskCol[1] << "\">"
//   <<
//   // "Acrobot" << "</td></tr>" << endl; ofs << "<tr><td bgcolor=\"" <<
//   // taskCol[2] << "\">" << "CartCentering" << "</td></tr>" << endl; ofs <<
//   // "<tr><td bgcolor=\"" << taskCol[3] << "\">" << "Pendulum" <<
//   "</td></tr>"
//   // << endl; ofs << "<tr><td bgcolor=\"" << taskCol[4] << "\">" <<
//   // "MountainCar" << "</td></tr>" << endl; ofs << "<tr><td bgcolor=\"" <<
//   // taskCol[5] << "\">" << "MountainCarC." << "</td></tr>" << endl;
//   //
//   // ofs << "</table>>" << endl;
//   // ofs << ", fontsize=84, regular=1];" << endl;
//   // ofs << "}" << endl;
//   ///////////////////////////////////////////////////////////

//   ofs << "}" << endl;
//   ofs.close();
// }

/******************************************************************************/
void TPG::printGraphDotGPTPXXI(long rootTeamId,
                               set<team *, teamIdComp> &visitedTeamsAllTasks,
                               vector<map<long, double>> &teamUseMapPerTask,
                               vector<int> &steps_per_task) {
   team *rootTeam = _teamMap[rootTeamId];
   vector<string> taskCol;
   taskCol.push_back("#7fc97f");
   taskCol.push_back("#beaed4");
   taskCol.push_back("#fdc086");
   taskCol.push_back("#ffff99");
   // taskCol.push_back("#386cb0");
   // taskCol.push_back("#f0027f");
   map<long, string> nodeLabMap;
   double nodeWidth = 2.0;
   double arrowSize_1 = 3;  // 0.1;

   char outputFilename[80];
   ofstream ofs;

   sprintf(outputFilename, "replay/graphs/gv_taskDecomposition_%d%s",
           (int)rootTeam->id_, ".dot");
   ofs.open(outputFilename, ios::out);
   if (!ofs) die(__FILE__, __FUNCTION__, __LINE__, "Can't open file.");

   ofs << "strict digraph G {" << endl;
   ofs << "ratio=0.7" << endl;
   ofs << "root=t_" << rootTeam->id_ << endl;

   // teams
   for (auto tm : visitedTeamsAllTasks) {
      string col = "";
      ofs << " t_" << tm->id_ << " [shape=circle, style=wedged, fillcolor=\"";
      for (int tsk = 0; tsk < GetState("n_task"); tsk++) {
         ofs << taskCol[tsk] << ";";
         if (teamUseMapPerTask[tsk].find(tm->id_) !=
             teamUseMapPerTask[tsk].end()) {
            ofs << (teamUseMapPerTask[tsk][tm->id_] / steps_per_task[tsk]) /
                       GetState("n_task");
         } else {
            ofs << 0;
         }
         if (tsk < GetState("n_task") - 1) ofs << ":";
      }
      ofs << "\"";
      ofs << ", label=\"";
      if (nodeLabMap.find(tm->id_) == nodeLabMap.end()) {
         ofs << "";
      } else {
         ofs << nodeLabMap[tm->id_];
      }
      ofs << "\", fontsize=84, regular=1, width=" << nodeWidth * 2
          << ",penwidth=0]" << endl;
   }
   // team -> team edges
   for (auto tm : visitedTeamsAllTasks) {
      // auto mem = tm->members_;
      for (auto prog : tm->members_) {
         if (prog->action() >= 0 &&
             find(visitedTeamsAllTasks.begin(), visitedTeamsAllTasks.end(),
                  _teamMap[prog->action()]) != visitedTeamsAllTasks.end()) {
            ofs << " t_" << tm->id_ << "->t_" << prog->action()
                << " [arrowsize=" << arrowSize_1 << ", penwidth=" << "1"
                << " color=" << "black" << "];" << endl;
         }
      }
   }

   // //legend
   // ofs << "subgraph {" << endl;
   // ofs << "ratio=1" << endl;
   // ofs << "rank=sink" << endl;
   // ofs << "node [shape=plaintext]" << endl;
   // ofs << "legend [colorscheme=set18," << endl;
   // ofs << "label=<" << endl;
   // ofs << "<table border=\"0\" cellborder=\"1\" cellspacing=\"0\">" << endl;
   // ofs << "<tr><td bgcolor=\"" << taskCol[0] << "\">" << "CartPole" <<
   // "</td></tr>" << endl; ofs << "<tr><td bgcolor=\"" << taskCol[1] << "\">"
   // << "Pendulum" << "</td></tr>" << endl; ofs << "<tr><td bgcolor=\"" <<
   // taskCol[2] << "\">" << "Sunspots" << "</td></tr>" << endl; ofs <<
   // "<tr><td bgcolor=\"" << taskCol[3] << "\">" << "Mackey-Glass" <<
   // "</td></tr>" << endl; ofs << "</table>>" << endl; ofs << ", fontsize=84,
   // regular=1];" << endl; ofs << "}" << endl;

   ofs << "}" << endl;
   ofs.close();
}

// /******************************************************************************/
// void TPG::printGraphDotGPEMAnimate(
//     long rootTeamId, size_t frame, int episode, int step, size_t depth,
//     vector<program *> allPrograms, vector<program *> winningPrograms,
//     set<team *, teamIdComp> &visitedTeamsAllTasks,
//     vector<map<long, double>> &teamUseMapPerTask, vector<team *> teamPath) {
//   team *rootTeam = _teamMap[rootTeamId];

//   vector<string> taskCol;
//   taskCol.push_back("#7fc97f");
//   taskCol.push_back("#beaed4");
//   taskCol.push_back("#fdc086");
//   taskCol.push_back("#ffff99");
//   taskCol.push_back("#386cb0");
//   taskCol.push_back("#f0027f");
//   map<long, string> nodeLabMap;

//   nodeLabMap[1594815] = "1";  //   29000"
//   nodeLabMap[624659] = "2";   //  149"
//   nodeLabMap[1302870] = "3";  //   28373"
//   nodeLabMap[623346] = "4";   //  580"
//   nodeLabMap[493990] = "5";   //  149"
//   nodeLabMap[836830] = "6";   //  28019"
//   nodeLabMap[548151] = "7";   //  832"
//   nodeLabMap[126871] = "8";   //  1373"
//   nodeLabMap[425177] = "9";   //  774"
//   nodeLabMap[602173] = "10";  //   1826"
//   nodeLabMap[42314] = "11";   //  9"
//   nodeLabMap[26879] = "12";   //  7"
//   nodeLabMap[200127] = "13";  //   4"
//   nodeLabMap[470578] = "14";  //   1826"
//   nodeLabMap[5964] = "15";    // 7"
//   nodeLabMap[23266] = "16";   //  2"
//   nodeLabMap[226807] = "17";  //   394"
//   nodeLabMap[180005] = "18";  //   953"

//   bool drawPath = true;
//   double nodeWidth = 2.0;
//   double arrowSize_1 = 3;
//   double arrowSize_2 = 3;
//   double edgeWidth_2 = 10;
//   double edgeWidth_1 = 1;

//   char outputFilename[80];
//   ofstream ofs;

//   set<team *, teamIdComp> teams;
//   set<program *, programIdComp> programs;
//   set<memoryEigen *, memoryEigenIdComp> memories;
//   vector<program *> winningProgramsDepth(winningPrograms.begin(),
//                                          winningPrograms.end());

//   for (auto it = visitedTeamsAllTasks.begin(); it !=
//   visitedTeamsAllTasks.end();
//        it++) {
//     set<program *, programIdComp> p = (*it)->CopyMembers();  // no need to
//     copy programs.insert(p.begin(), p.end());
//   }
//   for (auto it = programs.begin(); it != programs.end(); it++) {
//     for (int mem_t = 0; mem_t < memoryEigen::NUM_MEMORY_TYPES; mem_t++) {
//       memories.insert((*it)->MemGet(mem_t));
//     }
//   }

//   sprintf(outputFilename, "replay/graphs/gv_%d_%05d_%03d_%05d_%05d%s",
//           (int)rootTeam->id_, (int)frame, episode, step, (int)depth, ".dot");
//   ofs.open(outputFilename, ios::out);
//   if (!ofs) die(__FILE__, __FUNCTION__, __LINE__, "Can't open file.");

//   ofs << "strict digraph G {" << endl;
//   ofs << "ratio=0.7" << endl;
//   ofs << "root=t_" << rootTeam->id_ << endl;

//   // programs
//   for (auto leiter = programs.begin(); leiter != programs.end(); leiter++) {
//     if (step > 0 &&
//         find(winningProgramsDepth.begin(), winningProgramsDepth.end(),
//              *leiter) != winningProgramsDepth.end()) {
//       // ofs << " p_" << (*leiter)->id_ << " [shape=box, style=filled,
//       // color=black, label=\"\", fontsize=200, regular=1, width=" <<
//       nodeWidth
//       // << "]" << endl;
//       ofs << " p_" << (*leiter)->id_
//           << " [shape=box, style=filled, color=green, label=\"\", "
//              "fontsize=200, regular=1, width="
//           << nodeWidth << "]" << endl;
//     } else if (step > 0 && find(allPrograms.begin(), allPrograms.end(),
//                                 *leiter) != allPrograms.end())
//       ofs << " p_" << (*leiter)->id_
//           << " [shape=box, style=filled, color=black, label=\"\", "
//              "fontsize=200, regular=1, width="
//           << nodeWidth << "]" << endl;
//     else if (teamPath.size() > 0)
//       ofs << " p_" << (*leiter)->id_
//           << " [shape=box, style=filled, color=grey90, label=\"\", "
//              "fontsize=200, regular=1, width="
//           << nodeWidth << "]" << endl;
//     else
//       ofs << " p_" << (*leiter)->id_
//           << " [shape=box, style=filled, color=grey70, label=\"\", "
//              "fontsize=200, regular=1, width="
//           << nodeWidth << "]" << endl;
//   }

//   // teams
//   for (auto teiter = visitedTeamsAllTasks.begin();
//        teiter != visitedTeamsAllTasks.end(); teiter++) {
//     string col = "";

//     ofs << " t_" << (*teiter)->id_
//         << " [shape=circle, style=wedged, fillcolor=\"";
//     double sumUse = 0;
//     for (int tsk = 0; tsk < 6; tsk++) {
//       if (teamUseMapPerTask[tsk].find((*teiter)->id_) !=
//           teamUseMapPerTask[tsk].end())
//         sumUse += teamUseMapPerTask[tsk][(*teiter)->id_];
//     }
//     for (int tsk = 0; tsk < 6; tsk++) {
//       ofs << taskCol[tsk] << ";"
//           << (teamUseMapPerTask[tsk].find((*teiter)->id_) !=
//                       teamUseMapPerTask[tsk].end()
//                   ? teamUseMapPerTask[tsk][(*teiter)->id_] / sumUse
//                   : 0);
//       if (tsk < 5) ofs << ":";
//     }
//     ofs << "\"";

//     ofs << ", label=\""
//         << (nodeLabMap.find((*teiter)->id_) == nodeLabMap.end()
//                 ? ""
//                 : nodeLabMap[(*teiter)->id_])
//         << "\", fontsize=84, regular=1, width=" << nodeWidth * 2
//         << ",penwidth=0]" << endl;
//   }

//   ////team -> team edges
//   // for(auto teiter = visitedTeamsAllTasks.begin(); teiter !=
//   // visitedTeamsAllTasks.end(); teiter++){
//   //    list < program * > mem;
//   //    (*teiter)->members(&mem);
//   //    for(auto leiter = mem.begin(); leiter != mem.end(); leiter++){
//   //       if ((*leiter)->action() >= 0 && find(visitedTeamsAllTasks.begin(),
//   //       visitedTeamsAllTasks.end(), _teamMap[(*leiter)->action()]) !=
//   //       visitedTeamsAllTasks.end()){
//   //          ofs << " t_" << (*teiter)->id_ << "->t_" << (*leiter)->action()
//   //          << " [arrowsize=" << arrowSize_1  << ", penwidth=" << "1" << "
//   //          color=" << "black" << "];" << endl;
//   //       }
//   //    }
//   // }

//   ////team -> team edges path
//   // if (step > 0){
//   //    for(size_t t = 0; t < teamPath.size()-1; t++){
//   //       list < program * > mem;
//   //       teamPath[t]->members(&mem);
//   //       for(auto leiter = mem.begin(); leiter != mem.end(); leiter++){
//   //          if ((*leiter)->action() >= 0 && teamPath[t+1]->id_ ==
//   //          _teamMap[(*leiter)->action()]->id_){
//   //             ofs << " t_" << teamPath[t]->id_ << "->t_" <<
//   //             (*leiter)->action() << " [arrowsize=" << arrowSize_2  << ",
//   //             penwidth=" << edgeWidth_2 << " color=" << "black" << "];" <<
//   //             endl;
//   //          }
//   //       }
//   //    }
//   // }

//   // program -> team edges
//   for (auto leiter = programs.begin(); leiter != programs.end(); leiter++) {
//     if ((*leiter)->action() >= 0 &&
//         find(visitedTeamsAllTasks.begin(), visitedTeamsAllTasks.end(),
//              _teamMap[(*leiter)->action()]) != visitedTeamsAllTasks.end()) {
//       double w = find(winningProgramsDepth.begin(),
//       winningProgramsDepth.end(),
//                       *leiter) == winningProgramsDepth.end() ||
//                          !drawPath
//                      ? edgeWidth_1
//                      : edgeWidth_2;
//       if (teamPath.size() < 1) w = 5;
//       double as = find(winningProgramsDepth.begin(),
//       winningProgramsDepth.end(),
//                        *leiter) == winningProgramsDepth.end() ||
//                           !drawPath
//                       ? arrowSize_1
//                       : arrowSize_2;
//       string col =
//           find(winningProgramsDepth.begin(), winningProgramsDepth.end(),
//                *leiter) == winningProgramsDepth.end()
//               ? "black"
//               : "green";
//       ofs << " p_" << (*leiter)->id_ << "->"
//           << "t_" << (*leiter)->action() << " [arrowsize=" << as
//           << ", penwidth=" << w << " color=" << col.c_str() << "];" << endl;
//     }
//   }

//   // team -> program edges
//   for (auto teiter = visitedTeamsAllTasks.begin();
//        teiter != visitedTeamsAllTasks.end(); teiter++) {
//     list<program *> mem;
//     (*teiter)->members(&mem);
//     for (auto leiter = mem.begin(); leiter != mem.end(); leiter++) {
//       double w =
//           find(teamPath.begin(), teamPath.end(), *teiter) == teamPath.end()
//           ||
//                   !drawPath
//               ? edgeWidth_1
//               : edgeWidth_2;
//       if (teamPath.size() < 1) w = 5;
//       double as =
//           find(teamPath.begin(), teamPath.end(), *teiter) == teamPath.end()
//           ||
//                   !drawPath
//               ? arrowSize_1
//               : arrowSize_2;
//       string col =
//           find(winningProgramsDepth.begin(), winningProgramsDepth.end(),
//                *leiter) == winningProgramsDepth.end()
//               ? "black"
//               : "green";
//       ofs << " t_" << (*teiter)->id_ << "->p_" << (*leiter)->id_
//           << " [arrowsize=" << as << ", penwidth=" << w
//           << " color=" << col.c_str() << "];" << endl;
//     }
//   }

//   ofs << "}" << endl;
//   ofs.close();
// }

/******************************************************************************/
void TPG::printPhyloGraphDot(team *tm) {
   char outputFilename[80];
   ofstream ofs;

   sprintf(outputFilename, "replay/graphs/phylo-t%05d-s%lu%s",
           (int)GetState("t_current"), seeds_[TPG_SEED], ".dot");
   ofs.open(outputFilename, ios::out);
   if (!ofs) die(__FILE__, __FUNCTION__, __LINE__, "Can't open file.");

   ofs << "digraph G {" << endl;

   // Basic breadth-first search

   std::vector<long> visited = {tm->id_};
   list<long> queue = {tm->id_};

   while (!queue.empty()) {
      long currId = queue.front();
      queue.pop_front();

      for (long ancId : _phyloGraph[currId].ancestorIds) {
         ofs << ancId << " -> " << currId << endl;
         if (std::find(visited.begin(), visited.end(), ancId) ==
             visited.end()) {
            visited.push_back(ancId);
            queue.push_back(ancId);
         }
      }
   }

   // Color nodes based on fitness
   for (long id : visited) {
      double fitness = _phyloGraph[id].fitness;
      double hue = std::clamp(fitness, 0.0, 1.0) / 3;

      ofs << id << " [label=\"id: " << id << "\\nfit: " << std::fixed
          << std::setprecision(4) << fitness << "\" style=filled, fillcolor=\""
          << hue << " 1.000 1.000\"]" << endl;
   }

   ofs << "}" << endl;
   ofs.close();
}

/******************************************************************************/
void TPG::printTeamInfo(long t, int phase, bool singleBest, long teamId) {
   team *bestTeam = *(_Mroot.begin());
   if (singleBest && teamId == -1) bestTeam = getBestTeam();
   ostringstream tmposs;
   map<point *, double, pointLexicalLessThan> allOutcomes;
   // map < point *, double > :: iterator myoiter;
   vector<int> behaviourSequence;
   set<team *, teamIdComp> visitedTeams;
   for (auto teiter = _M.begin(); teiter != _M.end(); teiter++) {
      if ((!singleBest && (*teiter)->root() &&
           teamId == -1) ||                             // all root teams
          (!singleBest && (*teiter)->id_ == teamId) ||  // specific team
          (singleBest &&
           (*teiter)->id_ == bestTeam->id_))  // singleBest root team
      {
         oss << "tminfo t " << t << " id " << (*teiter)->id_ << " gtm "
             << (*teiter)->gtime_ << " phs " << phase;
         oss << " root " << ((*teiter)->root() ? 1 : 0);
         oss << " sz " << (*teiter)->size();
         oss << " age " << t - (*teiter)->gtime_;
         // oss << " compl";
         // oss << " " << (*teiter)->numActiveTeams_;
         // oss << " " << (*teiter)->numActivePrograms_;
         // oss << " " << (*teiter)->numEffectiveInstructions();
         // oss << " " << (*teiter)->numActiveFeatures_;
         oss << " nOut " << (*teiter)->numOutcomes(_TRAIN_PHASE, -1) << " "
             << (*teiter)->numOutcomes(_TEST_PHASE, -1);

         oss << setprecision(5) << fixed;

         oss << " mnOut";
         for (int phs : {0, 1, 2}) {
            // bool allPhase = false;
            // bool allTask = false;  // for genomic, set to true

            // multi-task
            // statistics stored in aux doubles
            for (int task = 0; task < GetState("n_task"); task++) {
               for (int i = 0; i < GetParam<int>("n_point_aux_double"); i++) {
                  if ((*teiter)->numOutcomes(phs, task) > 0) {
                     oss << " p" << phs << "t" << task << "a" << i << " ";
                     // oss << (*teiter)->getQuickMean(task,
                     // GetState("fitMode"), phs);
                     oss << (*teiter)->getMeanOutcome(phs, task, i, false,
                                                      false);
                  } else
                     oss << " p" << phs << "t" << task << "a" << i << " x";
               }
            }
         }

         oss << " fit " << (*teiter)->fit_;

         oss << setprecision(2) << fixed;

         visitedTeams.clear();
         vector<int> programInstructionCounts,
             effectiveProgramInstructionCounts;
         (*teiter)->policyInstructions(_teamMap, visitedTeams,
                                       programInstructionCounts,
                                       effectiveProgramInstructionCounts);

         oss << " pIns "
             << accumulate(programInstructionCounts.begin(),
                           programInstructionCounts.end(), 0);
         oss << " mnProgIns " << vecMean(programInstructionCounts);

         oss << " ePIns "
             << accumulate(effectiveProgramInstructionCounts.begin(),
                           effectiveProgramInstructionCounts.end(), 0);
         oss << " mnEProgIns " << vecMean(effectiveProgramInstructionCounts);
         set<program *, programIdComp> programs;
         // set<memoryEigen *, memoryEigenIdComp> memories;
         set<team *, teamIdComp> visitedTeams2;
         (*teiter)->GetAllNodes(_teamMap, visitedTeams2, programs);
         oss << " nP " << programs.size();
         oss << " nT " << visitedTeams2.size();
         // oss << " nM " << memories.size();

         // visitedTeams.clear();
         // set<long> pF;
         // (*teiter)->policyFeatures(_teamMap, visitedTeams, pF, true);
         // oss << " pF " << (double)pF.size() / n_input_[];
         // //GetParam<int>("n_input");

         vector<int> op_countsSingle;
         vector<int> op_countsTally;
         op_countsTally.resize(instruction::NUM_OP);

         fill(op_countsTally.begin(), op_countsTally.end(), 0);
         for (auto it = programs.begin(); it != programs.end(); it++) {
            (*it)->op_counts(op_countsSingle);
            for (size_t i = 0; i < op_countsSingle.size(); i++)
               op_countsTally[i] += op_countsSingle[i];
         }
         oss << " nOp " << vecToStr(op_countsTally);

         vector<int> tmSizesRoot, tmSizesSub;
         tmSizesRoot.push_back((*teiter)->size());
         for (auto teiter2 = visitedTeams2.begin();
              teiter2 != visitedTeams2.end(); teiter2++)
            if ((*teiter2)->id_ !=
                (*teiter)->id_)  // not the root of this policy
               tmSizesSub.push_back((*teiter2)->size());
         oss << " mnTmSzR " << vecMean(tmSizesRoot) << " mnTmSzS "
             << (tmSizesSub.size() > 0 ? vecMean(tmSizesSub) : 0);

         ////map < int, map< int, map <point *, double, pointLexicalLessThan
         ///> > >
         /// outs;
         // map < int, map< int, map <int, point *> > > outs;
         //(*teiter)->outcomes(outs);

         // double maxVisitedTeams = 0;
         // for (auto ouiter = outs.begin(); ouiter != outs.end();
         // ouiter++){//tasks
         //   for (auto ouiter2 = ouiter->second[2].begin(); ouiter2 !=
         //   ouiter->second[2].end(); ouiter2++){//test outcomes, loop over
         //   envSeed
         //      maxVisitedTeams = max(maxVisitedTeams,
         //      ouiter2->second->auxDouble(2));
         //   }
         // }
         // oss << " maxVisitedTeams " << maxVisitedTeams << endl;

         // oss << " outcomes";
         // int nSolved = 0;
         // for(auto ouiter = outs[0][0].begin(); ouiter != outs[0][0].end();
         // ouiter++){
         //    if ((ouiter->first)->auxDouble(0) > 1)
         //       oss << " " << (ouiter->first)->auxDouble(0);
         //    if ((ouiter->first)->auxDouble(0) >= 50)
         //       nSolved++;
         // }
         // oss << " outcomesDesc";
         // for(auto ouiter = outs[0][0].begin(); ouiter != outs[0][0].end();
         // ouiter++){
         //    if ((ouiter->first)->auxDouble(0) >= 50)
         //       oss<< " " << (ouiter->first)->desc().c_str();
         // }
         // oss << " nSol " << nSolved;

         // oss << " fBins";
         // map <long, int> bins = (*teiter)->fitnessBins();
         // for (auto iter = bins.begin(); iter != bins.end(); iter++)
         //    oss << ":" << (*iter).first << "-" << (*iter).second;
         oss << endl;
      }
   }
}

void TPG::trackTeamInfo(long t, int phase, bool singleBest, long teamId) {
   string gen = std::to_string(t);

   team *bestTeam = *(_Mroot.begin());
   if (singleBest && teamId == -1) bestTeam = getBestTeam();
   ostringstream tmposs;
   map<point *, double, pointLexicalLessThan> allOutcomes;
   // map < point *, double > :: iterator myoiter;
   vector<int> behaviourSequence;
   set<team *, teamIdComp> visitedTeams;
   for (auto teiter = _M.begin(); teiter != _M.end(); teiter++) {
      if ((!singleBest && (*teiter)->root() &&
           teamId == -1) ||                             // all root teams
          (!singleBest && (*teiter)->id_ == teamId) ||  // specific team
          (singleBest &&
           (*teiter)->id_ == bestTeam->id_))  // singleBest root team
      {
         api_client_->LogMetric("teamInfo/root",
                                ((*teiter)->root() ? "1" : "0"), "", gen);
         api_client_->LogMetric("teamInfo/sz",
                                std::to_string((*teiter)->size()), "", gen);
         api_client_->LogMetric("teamInfo/age",
                                std::to_string(t - (*teiter)->gtime_), "", gen);
         api_client_->LogMetric(
             "teamInfo/nOutTrain",
             std::to_string((*teiter)->numOutcomes(_TRAIN_PHASE, -1)), "", gen);
         api_client_->LogMetric(
             "teamInfo/nOutTest",
             std::to_string((*teiter)->numOutcomes(_TEST_PHASE, -1)), "", gen);
         api_client_->LogMetric("teamInfo/fit", std::to_string((*teiter)->fit_),
                                "", gen);

         visitedTeams.clear();
         vector<int> programInstructionCounts,
             effectiveProgramInstructionCounts;
         (*teiter)->policyInstructions(_teamMap, visitedTeams,
                                       programInstructionCounts,
                                       effectiveProgramInstructionCounts);

         int pIns = accumulate(programInstructionCounts.begin(),
                               programInstructionCounts.end(), 0);
         double mnProgIns = vecMean(programInstructionCounts);
         int ePIns = accumulate(effectiveProgramInstructionCounts.begin(),
                                effectiveProgramInstructionCounts.end(), 0);
         double mnEProgIns = vecMean(effectiveProgramInstructionCounts);
         api_client_->LogMetric("teamInfo/pIns", std::to_string(pIns), "", gen);
         api_client_->LogMetric("teamInfo/mnProgIns", std::to_string(mnProgIns),
                                "", gen);
         api_client_->LogMetric("teamInfo/ePIns", std::to_string(ePIns), "",
                                gen);
         api_client_->LogMetric("teamInfo/mnEProgIns",
                                std::to_string(mnEProgIns), "", gen);

         set<program *, programIdComp> programs;
         // set<memoryEigen *, memoryEigenIdComp> memories;
         set<team *, teamIdComp> visitedTeams2;
         (*teiter)->GetAllNodes(_teamMap, visitedTeams2, programs);
         api_client_->LogMetric("teamInfo/nP", std::to_string(programs.size()),
                                "", gen);
         api_client_->LogMetric("teamInfo/nT",
                                std::to_string(visitedTeams2.size()), "", gen);
         // api_client_->LogMetric("teamInfo/nM",
         // std::to_string(memories.size()), "",
         //                        gen);

         vector<int> op_countsSingle;
         vector<int> op_countsTally;
         op_countsTally.resize(instruction::NUM_OP);

         fill(op_countsTally.begin(), op_countsTally.end(), 0);
         for (auto it = programs.begin(); it != programs.end(); it++) {
            (*it)->op_counts(op_countsSingle);
            for (size_t i = 0; i < op_countsSingle.size(); i++)
               op_countsTally[i] += op_countsSingle[i];
         }

         api_client_->LogMetric("teamInfo/nOp", vecToStr(op_countsTally), "",
                                gen);

         vector<int> tmSizesRoot, tmSizesSub;
         tmSizesRoot.push_back((*teiter)->size());
         for (auto teiter2 = visitedTeams2.begin();
              teiter2 != visitedTeams2.end(); teiter2++)
            if ((*teiter2)->id_ !=
                (*teiter)->id_)  // not the root of this policy
               tmSizesSub.push_back((*teiter2)->size());

         double mnTmSzR = vecMean(tmSizesRoot);
         int mnTmSzS = (tmSizesSub.size() > 0 ? vecMean(tmSizesSub) : 0);
         api_client_->LogMetric("teamInfo/mnTmSzR", std::to_string(mnTmSzR), "",
                                gen);
         api_client_->LogMetric("teamInfo/mnTmSzS", std::to_string(mnTmSzS), "",
                                gen);
      }
   }
}

/******************************************************************************/
// Algorithm 5.1 (linear crossover)
void TPG::programCrossover(RegisterMachine *p1, RegisterMachine *p2,
                           RegisterMachine **c1, RegisterMachine **c2,
                           mt19937 &rng) {
   int dcMax = min(p1->Size(), p2->Size());
   int dsMax = dcMax;
   int lsMax = dcMax;

   *c1 = dynamic_cast<RegisterMachine *>(CloneProgram(p1));
   *c2 = dynamic_cast<RegisterMachine *>(CloneProgram(p2));

   int pos1, pos2;

   vector<program *> parents{p1, p2};
   vector<int> segLengths{1, 1};

   if (p1->Size() > p2->Size()) swap(parents[0], parents[1]);

   // 1
   uniform_int_distribution<> dis1(0, parents[0]->Size() - 1);
   pos1 = dis1(rng);
   uniform_int_distribution<> dis2(0, parents[1]->Size() - 1);
   do {
      pos2 = dis2(rng);
   } while (abs(pos1 - pos2) > min(parents[0]->Size() - 1, dcMax));

   // 2,3
   uniform_int_distribution<> dis3(1, min(parents[0]->Size() - pos1, lsMax));
   segLengths[0] = dis3(rng);
   uniform_int_distribution<> dis4(1, min(parents[1]->Size() - pos2, lsMax));
   do {
      segLengths[1] = dis4(rng);
   } while (abs(segLengths[0] - segLengths[1]) > dsMax);

   // 4
   if (segLengths[0] > segLengths[1]) swap(segLengths[0], segLengths[1]);

   // 5
   if (p1->Size() - (segLengths[1] - segLengths[0]) < 1 ||
       p2->Size() + (segLengths[1] - segLengths[0]) >
           GetParam<int>("max_prog_size")) {
      if (real_dist_(rng) < 0.5)
         segLengths[1] = segLengths[0];
      else
         segLengths[0] = segLengths[1];

      if (pos1 + segLengths[0] > p1->Size())
         segLengths[0] = segLengths[1] = p1->Size() - pos1;
   }

   vector<instruction *> parentProg1 = p1->bid_;
   vector<instruction *> parentProg2 = p2->bid_;

   vector<instruction *> childProg1 = p1->bid_;
   vector<instruction *> childProg2 = p2->bid_;

   // exchange seg1 in p1 by seg2 in p2
   childProg1.clear();
   auto start = parentProg1.begin();
   auto end = parentProg1.begin() + pos1;
   copy(start, end, back_inserter(childProg1));
   start = parentProg2.begin() + pos2;
   end = parentProg2.begin() + pos2 + segLengths[1];
   copy(start, end, back_inserter(childProg1));
   start = parentProg1.begin() + pos1 + segLengths[0];
   end = parentProg1.end();
   copy(start, end, back_inserter(childProg1));

   // exchange seg2 in p2 by seg1 in p1
   childProg2.clear();
   start = parentProg2.begin();
   end = parentProg2.begin() + pos2;
   copy(start, end, back_inserter(childProg2));
   start = parentProg1.begin() + pos1;
   end = parentProg1.begin() + pos1 + segLengths[0];
   copy(start, end, back_inserter(childProg2));
   start = parentProg2.begin() + pos2 + segLengths[1];
   end = parentProg2.end();
   copy(start, end, back_inserter(childProg2));
}

/******************************************************************************/
// Read in populations from a checkpoint file.
// TODO(skelly): move reading logic to "deserialize" constructors and cleanup
void TPG::ReadCheckpoint(long t, int phase, int chkpID, bool fromString,
                         const string &inString) {
   finalize();  // clear populations

   string str;

   if (fromString) {
      str = inString;
   } else {
      char filename[80];
      sprintf(filename, "%s/%s.%ld.%d.%lu.%d.rslt", "checkpoints", "cp", t,
              chkpID, seeds_[TPG_SEED], phase);
      ifstream t(filename);
      t.seekg(0, ios::end);
      str.reserve(t.tellg());
      t.seekg(0, ios::beg);
      str.assign((istreambuf_iterator<char>(t)), istreambuf_iterator<char>());
   }

   istringstream iss(str);
   string oneline;
   char delim = ':';
   long memberId = 0;
   long max_teamCount = -1;
   long max_programCount = -1;
   long max_memoryCount = -1;
   int f;

   vector<string> outcomeFields;

   while (getline(iss, oneline)) {
      if (oneline.size() == 0) continue;
      outcomeFields.clear();

      SplitString(oneline, delim, outcomeFields);

      if (outcomeFields[0].compare("teamPair") == 0) {
         long id1 = atoi(outcomeFields[1].c_str());
         long id2 = atoi(outcomeFields[2].c_str());
         teamPair tp(_teamMap[id1], _teamMap[id2]);
         _teamPairsToCompair.push_back(tp);
      } else if (outcomeFields[0].compare("t") == 0)
         state_["t_current"] = atoi(outcomeFields[1].c_str());
      else if (outcomeFields[0].compare("active_task") == 0)
         state_["active_task"] = atoi(outcomeFields[1].c_str());
      else if (outcomeFields[0].compare("fitMode") == 0)
         params_["fit_mode"] = atoi(outcomeFields[1].c_str());
      else if (outcomeFields[0].compare("phase") == 0)
         state_["phase"] = atoi(outcomeFields[1].c_str());
      else if (outcomeFields[0].compare("memoryEigen") == 0) {
         max_memoryCount =
             std::max(max_memoryCount, atol(outcomeFields[1].c_str()));
         AddMemory(new memoryEigen(outcomeFields));
      } else if (outcomeFields[0].compare("RegisterMachine") == 0) {
         max_programCount =
             std::max(max_programCount, atol(outcomeFields[1].c_str()));
         AddProgram(new RegisterMachine(outcomeFields, _Memory, params_,
                                        rngs_[TPG_SEED]));
      } else if (outcomeFields[0].compare("team") == 0) {
         team *m;
         f = 1;
         long id = atoi(outcomeFields[f++].c_str());
         if (id > max_teamCount) max_teamCount = id;
         long gtime = atoi(outcomeFields[f++].c_str());
         m = new team(gtime, id);
         m->_n_eval = atoi(outcomeFields[f++].c_str());
         // add programs in order
         for (size_t ii = f; ii < outcomeFields.size(); ii++) {
            memberId = atoi(outcomeFields[ii].c_str());
            m->AddProgram(_L[memberId]);
         }
         AddTeam(m);
      } else if (outcomeFields[0].compare("incoming_progs") == 0) {
         f = 1;
         long id = atoi(outcomeFields[f++].c_str());
         for (size_t ii = f; ii < outcomeFields.size(); ii++) {
            long incomingId = atoi(outcomeFields[ii].c_str());
            _teamMap[id]->AddIncomingProgram(incomingId);
         }
         _Mroot.erase(_teamMap[id]);
      } else if (!fromString && outcomeFields[0].compare("phyloNode") == 0 &&
                 find(outcomeFields.begin(), outcomeFields.end(), "gtime") ==
                     outcomeFields.end()) {
         f = 1;
         long id = atoi(outcomeFields[f++].c_str());
         _phyloGraph.insert(pair<long, phyloRecord>(id, phyloRecord()));
         _phyloGraph[id].gtime = atoi(outcomeFields[f++].c_str());
         _phyloGraph[id].dtime = atoi(outcomeFields[f++].c_str());
         _phyloGraph[id].fitnessBin = atoi(outcomeFields[f++].c_str());
         _phyloGraph[id].fitness = atof(outcomeFields[f++].c_str());
         _phyloGraph[id].root =
             atoi(outcomeFields[f++].c_str()) > 0 ? true : false;
      } else if (!fromString && outcomeFields[0].compare("phyloLink") == 0 &&
                 find(outcomeFields.begin(), outcomeFields.end(), "from") ==
                     outcomeFields.end())
         _phyloGraph[atoi(outcomeFields[1].c_str())].adj.push_back(
             atoi(outcomeFields[2].c_str()));
      else if (!fromString && outcomeFields[0].compare("ancestorIds") == 0) {
         f = 1;
         long id = atoi(outcomeFields[f++].c_str());
         for (size_t ii = f; ii < outcomeFields.size(); ii++) {
            long aid = atoi(outcomeFields[ii].c_str());
            _phyloGraph[id].ancestorIds.insert(aid);
         }
      }
   }
   state_["program_count"] = max_programCount + 1;
   state_["team_count"] = max_teamCount + 1;
   state_["memory_count"] = max_memoryCount + 1;
}

/******************************************************************************/
void TPG::recalculateProgramRefs() {
   for (auto p : _L) p.second->nrefs_ = 0;
   for (auto tm : _M)
      for (auto p : tm->members_) p->nrefs_++;
}

/******************************************************************************/
void TPG::SanityCheck() { TeamSizesMatchProgRefs(); }

/******************************************************************************/
void TPG::TeamSizesMatchProgRefs() {
   int sum_team_sizes = 0;
   int sum_prog_refs = 0;
   for (auto tm : _M) sum_team_sizes += tm->size();
   for (auto prog : _L) sum_prog_refs += prog.second->nrefs_;
   if (sum_prog_refs != sum_team_sizes) {
      std::string error_message =
          "Program reference mismatch. sum_team_sizes " +
          to_string(sum_team_sizes) + " sum_prog_refs " +
          to_string(sum_prog_refs);
      die(__FILE__, __FUNCTION__, __LINE__, error_message.c_str());
   }
}

/******************************************************************************/
void TPG::SelectTeams() {
   set<team *, teamFitnessLexicalCompare> teams;
   int numOldDeleted = 0;
   int numDeleted = 0;

   deque<program *> programsWithNoRefs;

   for (auto teiter = _Mroot.begin(); teiter != _Mroot.end();) {
      if (!(*teiter)->elite(GetState("phase")) &&
          !isElitePS(*teiter, GetState("phase"))) {
         if ((*teiter)->gtime_ < GetState("t_current")) numOldDeleted++;
         _phyloGraph[(*teiter)->id_].dtime = GetState("t_current");
         RemoveTeam(*teiter, programsWithNoRefs);
         teiter = _Mroot.erase(teiter);
         numDeleted++;
      } else
         teiter++;
   }

   CleanupProgramsWithNoRefs(programsWithNoRefs, false);

   oss << "selTms t " << GetState("t_current") << " Msz " << _M.size()
       << " Lsz " << _L.size() << " mrSz " << _Mroot.size() << " mSz";
   for (int mem_t = 0; mem_t < memoryEigen::NUM_MEMORY_TYPES; mem_t++) {
      oss << " " << _Memory[mem_t].size();
   }
   oss << " eLSz " << _numEliteTeamsCurrent[GetState("phase")] << " nDel "
       << numDeleted << " nOldDel " << numOldDeleted << " nOldDelPr "
       << (double)numOldDeleted / numDeleted;
   oss << endl;
}

/******************************************************************************/
void TPG::CleanupProgramsWithNoRefs(deque<program *> &programsWithNoRefs,
                                    bool updateLidsImmediately) {
   vector<long> deletedIds;
   while (programsWithNoRefs.size() > 0) {
      auto prog = programsWithNoRefs.front();
      if (prog->nrefs_ > 0) {
         programsWithNoRefs.pop_front();
         continue;
      }
      if (prog->action() >= 0) {
         if (_teamMap[prog->action()]->inDeg() == 1) {
            _Mroot.insert(_teamMap[prog->action()]);
            _phyloGraph[_teamMap[prog->action()]->id_].root = true;
         }
         _teamMap[prog->action()]->removeIncomingProgram(prog->id_);
         // if team was a subsumed root clone that has now become a root
         // itself, just delete it
         if (_teamMap[prog->action()]->root() &&
             _teamMap[prog->action()]->cloneId_ != -1) {
            auto it = _teamMap.find(_teamMap[prog->action()]->cloneId_);
            if (it != _teamMap.end())
               it->second->clones_ = it->second->clones_ - 1;
            team *tm = _teamMap[prog->action()];
            _phyloGraph[tm->id_].dtime = GetState("t_current");
            _Mroot.erase(tm);
            RemoveTeam(tm, programsWithNoRefs);
         }
      }
      // TODO(spkelly): remove shared memory code
      // for (int mem_t = 0; mem_t < memoryEigen::NUM_MEMORY_TYPES; mem_t++) {
      //   prog->MemGet(mem_t)->refDec();
      //   if (prog->MemGet(mem_t)->refs() == 0) {
      //     removeMemory(prog->MemGet(mem_t));
      //     delete prog->MemGet(mem_t);
      //   }
      // }
      removeProgram(prog, updateLidsImmediately);
      if (!updateLidsImmediately) deletedIds.push_back(prog->id_);
      delete prog;
      programsWithNoRefs.pop_front();
   }
   if (!updateLidsImmediately) {
      sort(deletedIds.begin(), deletedIds.end());
      sort(_Lids.begin(), _Lids.end());
      vector<long> diff;
      set_difference(_Lids.begin(), _Lids.end(), deletedIds.begin(),
                     deletedIds.end(), inserter(diff, diff.begin()));
      _Lids = diff;
   }
}

/******************************************************************************/
void TPG::teamTaskRank(int phase, const vector<int> &objectives) {
   oss << "TPG::teamTaskRank <team:avgRank>";
   for (auto teiterA = _M.begin(); teiterA != _M.end(); teiterA++) {
      if (!(*teiterA)->root()) continue;
      vector<int> ranks;
      for (size_t i = 0; i < objectives.size(); i++) ranks.push_back(1);
      for (size_t o = 0; o < objectives.size(); o++) {
         for (auto teiterB = _M.begin(); teiterB != _M.end(); teiterB++) {
            if (!(*teiterB)->root()) continue;
            if ((*teiterB)->getMeanOutcome(phase, objectives[o],
                                           GetParam<int>("fit_mode"), false,
                                           false) >
                (*teiterA)->getMeanOutcome(phase, objectives[o],
                                           GetParam<int>("fit_mode"), false,
                                           false))
               ranks[o]++;
         }
      }

      double rankSum = 0;
      for (size_t i = 0; i < ranks.size(); i++) rankSum += ranks[i];
      (*teiterA)->fit_ = 1 / (rankSum / ranks.size());
      oss << " " << (*teiterA)->id_ << ":" << (*teiterA)->fit_;
   }
   oss << endl;
}

/******************************************************************************/
void TPG::updateMODESFilters(bool roots) {
   (void)roots;
   vector<long> symbiontIntersection;
   symbiontIntersection.reserve(100);
   vector<long> symbiontUnion;
   symbiontUnion.reserve(100);

   if (GetState("t_current") != GetState("t_start")) {
      _persistenceFilterA.clear();
      for (auto teiter = _Mroot.begin(); teiter != _Mroot.end(); teiter++) {
         vector<long> ancestorIds;
         (*teiter)->getAncestorIds(ancestorIds);
         for (auto pr = _allComponentsA.begin(); pr != _allComponentsA.end();
              pr++)
            if (find(ancestorIds.begin(), ancestorIds.end(), (*pr).first) !=
                    ancestorIds.end() &&
                (*teiter)->hasOutcome(0, _TRAIN_PHASE, 0)) {
               // found an ancestor of *teiter in _allComponentsA, add
               // *teiter to _persistenceFilterA
               _persistenceFilterA.insert(
                   pair<long, modesRecord>((*teiter)->id_, modesRecord()));
               // store active programs for novelty metric
               set<team *, teamIdComp> teams;
               set<program *, programIdComp> programs;
               // set<memoryEigen *, memoryEigenIdComp> memories;
               (*teiter)->GetAllNodes(_teamMap, teams, programs);
               for (auto leiter = programs.begin(); leiter != programs.end();
                    leiter++) {
                  _persistenceFilterA[(*teiter)->id_].activeProgramIds.insert(
                      (*leiter)->id_);
                  _persistenceFilterA[(*teiter)->id_]
                      .effectiveInstructionsTotal += (*leiter)->SizeEffective();
               }
               for (auto teiter2 = teams.begin(); teiter2 != teams.end();
                    teiter2++)
                  _persistenceFilterA[(*teiter)->id_].activeTeamIds.insert(
                      (*teiter2)->id_);
               _persistenceFilterA[(*teiter)->id_].runTimeComplexityIns =
                   (*teiter)->runTimeComplexityIns();
               _persistenceFilterA[(*teiter)->id_].behaviourString =
                   (*teiter)->getBehaviourString(0, _TRAIN_PHASE);
               break;
            }
      }

      // do analysis here
      int change = 0;
      // double minNCD = 0;
      double novelty = 0;
      int complexityRTC = 0;
      int complexityTeams = 0;
      int complexityPrograms = 0;
      int complexityInstructions = 0;
      double ecology = 0;
      for (auto prA = _persistenceFilterA.begin();
           prA != _persistenceFilterA.end(); prA++) {
         // change
         if (_persistenceFilterA_t.find((*prA).first) ==
             _persistenceFilterA_t.end())
            change++;
         // novelty
         bool found = false;
         for (auto prAll = _persistenceFilterAllTime.begin();
              prAll != _persistenceFilterAllTime.end(); prAll++) {
            // double ncd =
            // normalizedCompressionDistance((*prAll).second.behaviourString,
            // (*prA).second.behaviourString);
            if ((*prAll).second.behaviourString.compare(
                    (*prA).second.behaviourString) == 0) {
               found = true;
               break;
            }
         }
         if (!found) novelty++;

         // complexity
         complexityRTC =
             max(complexityRTC, (int)(*prA).second.runTimeComplexityIns);
         complexityTeams =
             max(complexityTeams,
                 (int)(*prA).second.activeTeamIds.size());  // transitions ?
         complexityPrograms = max(complexityPrograms,
                                  (int)(*prA).second.activeProgramIds.size());
         complexityInstructions =
             max(complexityInstructions,
                 (int)(*prA).second.effectiveInstructionsTotal);

         // ecology
         double Pc = 0;
         for (auto prA2 = _persistenceFilterA.begin();
              prA2 != _persistenceFilterA.end(); prA2++)
            if ((*prA2).first != (*prA).first &&
                (*prA2).second.behaviourString.compare(
                    (*prA).second.behaviourString) == 0)
               Pc++;
         if (Pc > 0) {
            Pc = Pc / _persistenceFilterA.size();
            ecology += Pc * log2(Pc);
         }
      }
      ecology *= -1;

      oss << "TPG::MODES t " << GetState("t_current") << " pfASize "
          << _persistenceFilterA.size() << " pfA_tSize "
          << _persistenceFilterA_t.size();
      oss << " change " << change << " novelty " << novelty;
      oss << " complexityRTC " << complexityRTC << " complexityTeams "
          << complexityTeams << " complexityPrograms " << complexityPrograms;
      oss << " complexityInstructions " << complexityInstructions << " ecology "
          << ecology << endl;

      _persistenceFilterA_t.clear();
      _persistenceFilterA_t.insert(_persistenceFilterA.begin(),
                                   _persistenceFilterA.end());
      _persistenceFilterAllTime.insert(_persistenceFilterA.begin(),
                                       _persistenceFilterA.end());
   }

   // for next t
   _allComponentsA.clear();
   // for(auto teiter = _M.begin(); teiter != _M.end(); teiter++)
   //    if (!roots || (roots && (*teiter)->root()))
   for (auto teiter = _Mroot.begin(); teiter != _Mroot.end(); teiter++)
      _allComponentsA.insert(
          pair<long, modesRecord>((*teiter)->id_, modesRecord()));
}

/******************************************************************************/
void TPG::WriteCheckpoint(long t, bool elite) {
   ofstream ofs;
   char filename[80];
   sprintf(filename, "%s/%s.%ld.%d.%lu.%d.rslt", "checkpoints", "cp", t,
           GetParam<int>("id"), seeds_[TPG_SEED], GetState("phase"));
   ofs.open(filename, ios::out);
   if (!ofs.is_open() || ofs.fail()) {
      cerr << "open failed for file: " << filename
           << " error:" << strerror(errno) << '\n';
      die(__FILE__, __FUNCTION__, __LINE__, "Can't open file.");
   }
   ofs << "seed_tpg:" << seeds_[TPG_SEED] << endl;
   ofs << "seed_aux:" << seeds_[AUX_SEED] << endl;
   ofs << "t:" << GetState("t_current") << endl;
   ofs << "active_task:" << GetState("active_task") << endl;
   ofs << "fitMode:" << GetParam<int>("fit_mode") << endl;

   if (elite) {  // Include data for elite teams only
      set<memoryEigen *, memoryEigenIdComp> memories;
      set<program *, programIdComp> programs;
      set<team *, teamIdComp> teams, teamsAll;
      // Collect memories, progrms, teams in champions...
      for (auto ps : _eliteTeamPS) {  // ...for each task set
         for (auto fm : ps.second) {  // ...for each fitness mode
            auto tm = fm.second[GetState("phase")];
            teams.clear();
            tm->GetAllNodes(_teamMap, teams, programs, memories);
            teamsAll.insert(teams.begin(), teams.end());
         }
      }
      for (auto mem : memories) {
         ofs << mem->checkpoint();
      }
      for (auto prog : programs) {
         ofs << prog->checkpoint(GetParam<int>("skip_introns"));
      }
      for (auto tm : teamsAll) {
         ofs << tm->checkpoint();
      }
   } else {  // Include all memories, teams, and programs
      for (int mem_t = 0; mem_t < memoryEigen::NUM_MEMORY_TYPES; mem_t++) {
         for (auto key : _Memory[mem_t]) {
            ofs << key.second->checkpoint();
         }
      }
      for (auto key : _L) {
         ofs << key.second->checkpoint(false);  // Write all instructions
      }
      for (auto tm : _M) {
         ofs << tm->checkpoint();
      }
      ofs << SerializePhylogeny();
   }
   ofs << "end" << endl;
   ofs.close();
}

/******************************************************************************/
std::string TPG::SerializePhylogeny() {
   std::stringstream ss;
   ss << "phyloNode:id:gtime:dtime:fitness:root" << endl;
   ss << "phyloLink:from,to" << endl;
   for (auto it = _phyloGraph.begin(); it != _phyloGraph.end(); it++) {
      ss << "phyloNode:" << (*it).first << ":" << (*it).second.gtime << ":"
         << (*it).second.dtime << ":" << (*it).second.fitnessBin << ":"
         << (*it).second.fitness << ":" << (*it).second.root << endl;
      if ((*it).second.adj.size() > 0)
         for (size_t i = 0; i < (*it).second.adj.size(); i++)
            ss << "phyloLink:" << (*it).first << ":" << (*it).second.adj[i]
               << endl;
      if ((*it).second.ancestorIds.size() > 0) {
         ss << "ancestorIds:" << (*it).first;
         for (auto it2 = (*it).second.ancestorIds.begin();
              it2 != (*it).second.ancestorIds.end(); it2++)
            ss << ":" << *it2;
         ss << endl;
      }
   }
   return ss.str();
}

/******************************************************************************/
void TPG::WriteMPICheckpoint(string &s, vector<team *> &root_teams) {
   set<memoryEigen *, memoryEigenIdComp> memories;
   set<program *, programIdComp> programs;
   set<team *, teamIdComp> teams;
   for (auto tm : root_teams) {
      tm->GetAllNodes(_teamMap, teams, programs, memories);
   }
   stringstream ss;
   ss << "seed_tpg:" << seeds_[TPG_SEED] << endl;
   ss << "seed_aux:" << seeds_[AUX_SEED] << endl;
   ss << "t:" << GetState("t_current") << endl;
   ss << "active_task:" << GetState("active_task") << endl;
   ss << "fitMode:" << GetParam<int>("fit_mode") << endl;
   ss << "phase:" << GetState("phase") << endl;
   for (auto mem : memories) {
      ss << mem->checkpoint();
   }
   for (auto prog : programs) {
      ss << prog->checkpoint(GetParam<int>("skip_introns"));
   }
   for (auto tm : teams) {
      ss << tm->checkpoint();
   }
   ss << endl;
   s = ss.str();
}