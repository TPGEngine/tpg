#include "TPG.h"
#include <event_dispatcher.h>

/******************************************************************************/
TPG::TPG() {
   instruction::SetupOps();
   real_dist_ = uniform_real_distribution<>(0.0, 1.0);
   _Memids.resize(MemoryEigen::kNumMemoryType_);
   _Memory.resize(MemoryEigen::kNumMemoryType_);
   for (size_t i = 0; i < _NUM_PHASE; i++) _numEliteTeamsCurrent.push_back(0);
   _ops.resize(instruction::NUM_OP);
   fill(_ops.begin(), _ops.end(), false);
   rngs_.resize(NUM_RNG);
   seeds_.resize(NUM_RNG);
}

/******************************************************************************/
TPG::~TPG() { params_.clear(); }

/******************************************************************************/
void TPG::AddProgram(RegisterMachine *p) {
   program_pop_[p->id_] = p;
}

/******************************************************************************/
void TPG::AddTeam(team *tm) {
   team_pop_.insert(tm);
   _teamMap[tm->id_] = tm;
}

/******************************************************************************/
void TPG::RemoveTeam(team *tm) {
   // decrement program refs
   for (auto prog : tm->members_) {
      prog->nrefs_--;
   }
   // TODO(skelly): test cloning
   if (_teamMap.find(tm->cloneId_) != _teamMap.end())
      _teamMap[tm->cloneId_]->clones_--;
   _teamMap.erase(tm->id_);
   team_pop_.erase(tm);
   delete tm;
}

/******************************************************************************/
void TPG::AddMemory(long prog_id, MemoryEigen *m) {
   _Memory[m->type_][prog_id] = m;
}

/******************************************************************************/
team *TPG::getTeamByID(long id) {
   for (auto teiter = team_pop_.begin(); teiter != team_pop_.end(); teiter++)
      if ((*teiter)->id_ == id) return *teiter;
   return *(team_pop_.begin());
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
   for (size_t mem_t = 0; mem_t < MemoryEigen::kNumMemoryType_; mem_t++) {
      for (auto meiter = _Memory[mem_t].begin(); meiter != _Memory[mem_t].end();
           meiter++) {
         meiter->second->ClearWorking();
         meiter->second->ClearReadTime();
         meiter->second->ClearWriteTime();
      }
   }
}

/******************************************************************************/
RegisterMachine *TPG::getAction(team *tm, state *s, bool updateActive,
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
RegisterMachine *TPG::getAction(
    team *tm, state *s, bool updateActive,
    set<team *, teamIdComp> &visitedTeams, long &decisionInstructions,
    int timeStep, vector<RegisterMachine *> &allPrograms,
    vector<RegisterMachine *> &winningPrograms, vector<set<long>> &decisionFeatures,
    // vector<set<MemoryEigen *, MemoryEigenIdComp>> &decisionMemories,
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
                      set<RegisterMachine *, RegisterMachineIdComp> &programs) {
   teams.clear();
   programs.clear();
   tm->GetAllNodes(_teamMap, teams, programs);
}

/******************************************************************************/
map<long, team *> TPG::GetRootTeamsInMap() const {
   map<long, team *> teams;
   for (auto tm : team_pop_) {
      if (tm->root_) {
         teams[tm->id_] = tm;
      }
   }
   return teams;             
}

/******************************************************************************/
set<team*, teamFitnessLexicalCompare> TPG::GetRootTeamsInSet() {
   set<team*, teamFitnessLexicalCompare> teams;
   for (auto tm : team_pop_) {
      if (tm->root_) {
         teams.insert(tm);
      }
   }             
   return teams;
}

/******************************************************************************/
vector<team *> TPG::GetRootTeamsInVec() const {
   vector<team *> teams;
   for (auto tm : team_pop_) {
      if (tm->root_) {
         teams.push_back(tm);
      }
   }
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
   for (auto prog : program_pop_) {
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
   for (auto tm : team_pop_) {
      if (!roots || (roots && tm->root_)) {
         tm->resetOutcomes(phase);
      }
   }      
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
   _teamMap.clear();
   _Memids.clear();  //TODO(skelly): remove 
   _Memids.resize(MemoryEigen::kNumMemoryType_);
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

   for (auto tm : team_pop_) {
      tm->resetOutcomes(-1);
      delete tm;
   }
   team_pop_.clear();

   for (auto prog : program_pop_) {
      delete prog.second;
   }
   program_pop_.clear();

   for (auto memory_map : _Memory) {
      for (auto m : memory_map){ 
         delete m.second;
      }
   }

   _Memory.clear();
   _Memory.resize(MemoryEigen::kNumMemoryType_);

   phylo_graph_.clear();
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
      uniform_int_distribution<int> dis_programs(0, program_pop_.size() - 1);
      uniform_int_distribution<int> dis_team_size(0, team_to_mu->size() - 1);
      int random_prog_index = dis_programs(rngs_[TPG_SEED]);
      auto it = program_pop_.begin();
      std::advance(it, random_prog_index);
      RegisterMachine *p = it->second;
      team_to_mu->AddProgram(p, dis_team_size(rngs_[TPG_SEED]));
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
RegisterMachine *TPG::CloneProgram(RegisterMachine *prog) {
   RegisterMachine *prog_clone = new RegisterMachine(
       *(dynamic_cast<RegisterMachine *>(prog)), params_, state_);
   if (prog_clone->action_ >= 0)
      _teamMap[prog_clone->action_]->AddIncomingProgram(prog_clone->id_);
   return prog_clone;
}

/******************************************************************************/
void TPG::ProgramMutator_Instructions(RegisterMachine *prog_to_mu) {
   prog_to_mu->Mutate(params_, state_, rngs_[TPG_SEED], _ops);
}

/******************************************************************************/
void TPG::MutateActionToTerminal(RegisterMachine *prog_to_mu, team *new_team) {
   // If program is already terminal (action < 0) and there are no discrete
   // actions there is nothing to change
   if (prog_to_mu->action_ < 0 && GetParam<int>("n_discrete_action") == 0) {
      return;
   } else if (GetParam<int>("n_discrete_action") > 1) {
      uniform_int_distribution<int> dis(0,
                                        GetParam<int>("n_discrete_action") - 1);
      long new_discrete_action;
      do {
         // Discrete actions are negatives: -1 down to -n_discrete_action
         new_discrete_action = -1 - dis(rngs_[TPG_SEED]);
      } while (prog_to_mu->action_ == new_discrete_action);
      // If changing from team pointer to atomic, update pointee's incoming
      if (prog_to_mu->action_ >= 0) {
         _teamMap[prog_to_mu->action_]->removeIncomingProgram(prog_to_mu->id_);
      }
      prog_to_mu->action_ = new_discrete_action;
   }
}

/******************************************************************************/
void TPG::MutateActionToTeam(RegisterMachine *prog_to_mu, team *new_team,
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
                prog_to_mu->action_ == tm->id_));
      if (prog_to_mu->action_ >= 0)
         _teamMap[prog_to_mu->action_]->removeIncomingProgram(prog_to_mu->id_);
      if (!tm->root()) {  // Already subsumed, don't clone
         prog_to_mu->action_ = tm->id_;
         tm->AddIncomingProgram(prog_to_mu->id_);
      } else {  // clone when subsumed
         team *sub = new team(GetState("t_current"), state_["team_count"]++);
         tm->clone(phylo_graph_, &sub);
         prog_to_mu->action_ = sub->id_;
         sub->AddIncomingProgram(prog_to_mu->id_);
         // TODO(skelly): put in PhyloGraph functions
         phylo_graph_[tm->id_].adj.push_back(sub->id_);
         phylo_graph_.insert(pair<long, phyloRecord>(sub->id_, phyloRecord()));
         phylo_graph_[sub->id_].gtime = GetState("t_current");
         phylo_graph_[sub->id_].root = false;
         AddTeam(sub);
         n_new_teams++;
      }
   }
}

/******************************************************************************/
void TPG::ProgramMutator_ActionPointer(RegisterMachine *prog_to_mu, team *new_team,
                                       int &n_new_teams) {
   if (real_dist_(rngs_[TPG_SEED]) < GetParam<double>("pmn")) {
      if (real_dist_(rngs_[TPG_SEED]) < GetParam<double>("p_atomic")) {
         MutateActionToTerminal(prog_to_mu, new_team);
      } else {
         MutateActionToTeam(prog_to_mu, new_team, n_new_teams);
      }
   }
}

/******************************************************************************/
void TPG::AddAncestorToPhylogeny(team *parent, team *new_team) {
   phylo_graph_[new_team->id_].ancestorIds.insert(parent->id_);
   new_team->addAncestorId(parent->id_);
   phylo_graph_[parent->id_].adj.push_back(new_team->id_);
}

/******************************************************************************/
void TPG::AddTeamToPhylogeny(team *new_team) {
   phylo_graph_.insert(pair<long, phyloRecord>(new_team->id_, phyloRecord()));
   phylo_graph_[new_team->id_].gtime = GetState("t_current");
   phylo_graph_[new_team->id_].root = new_team->root_;
}

/******************************************************************************/
team* TPG::TeamCrossover(team* parent1, team* parent2) {
   team* child_team = new team(GetState("t_current"), state_["team_count"]++);
   // TODO(skelly): linear crossover
   if (parent1->size() == 1 && 
       parent2->size() == 1 &&
       parent1->members_.front()->instructions_.size() > 1 && 
       parent2->members_.front()->instructions_.size() > 1) {
      RegisterMachine* child_1;
      RegisterMachine* child_2;
      RegisterMachineCrossover(parent1->members_.front(),
                               parent2->members_.front(), &child_1, &child_2);
      if (real_dist_(rngs_[TPG_SEED]) < 0.5) {
         child_team->AddProgram(child_1);
         AddProgram(child_1);
         delete child_2;
      } else {
         child_team->AddProgram(child_2);
         AddProgram(child_2);
         delete child_1;
      }
   } else {
      std::list<RegisterMachine*> p1programs = parent1->members_;
      auto p1liter = p1programs.begin();
      std::list<RegisterMachine*> p2programs = parent2->members_;
      auto p2liter = p2programs.begin();
      // TODO(skelly): intertwine crossover
      while (p1liter != p1programs.end() || p2liter != p2programs.end()) {
         if (p1liter != p1programs.end()) {
            if ((*p1liter)->action_ < 0 && child_team->n_atomic_ < 1) {
               child_team->AddProgram(*p1liter);
            } else if ((int)child_team->size() <
                           GetParam<int>("max_team_size") &&
                       real_dist_(rngs_[TPG_SEED]) <
                           GetParam<double>("pmx_p")) {
               child_team->AddProgram(*p1liter);
            }
         }
         if (p2liter != p2programs.end()) {
            if ((*p2liter)->action_ < 0 && child_team->n_atomic_ < 1) {
               child_team->AddProgram(*p2liter);
            } else if ((int)child_team->size() <
                           GetParam<int>("max_team_size") &&
                       real_dist_(rngs_[TPG_SEED]) <
                           GetParam<double>("pmx_p")) {
               child_team->AddProgram(*p2liter);
            }
         }
         if (p1liter != p1programs.end())
            p1liter++;
         if (p2liter != p2programs.end())
            p2liter++;
      }
      if (child_team->n_atomic_ < 1) {
         die(__FILE__, __FUNCTION__, __LINE__,
             "Crossover must leave the fail-safe atomic program!");
      }
   }
   return child_team;
}

/******************************************************************************/
team* TPG::TeamSelector_Tournament(vector<team*>& candidate_parent_teams) {
   uniform_int_distribution<int> dis(0, candidate_parent_teams.size() - 1);
   auto tournament_size = GetParam<int>("tournament_size");
   auto tm = candidate_parent_teams[dis(rngs_[TPG_SEED])];
   while (tournament_size-- > 0) {
      auto i = dis(rngs_[TPG_SEED]);
      if (candidate_parent_teams[i]->fit_ > tm->fit_) {
         tm = candidate_parent_teams[i];
      }
   }
   return tm;
}

/******************************************************************************/
void TPG::GenerateNewTeams() {
   int new_teams_count = 0;
   auto task_power_set = PowerSet(GetState("n_task"));
   int n_new_teams_per_set =
       GetParam<int>("n_root_gen") / task_power_set.size();
   vector<team *> candidate_parent_teams;
   if (!GetParam<int>("parent_select_roots_only")) {
      candidate_parent_teams.resize(team_pop_.size());
      std::copy(team_pop_.begin(), team_pop_.end(), candidate_parent_teams.begin());
   }
   for (auto &subset : task_power_set) {
      //TODO(skelly): put selection in a separate function
      if (GetParam<int>("parent_select_roots_only")) {
         if (task_set_map_[vecToStrNoSpace(subset)].size() == 0) continue;
         candidate_parent_teams = task_set_map_[vecToStrNoSpace(subset)];
      }
      uniform_int_distribution<int> disP(0, candidate_parent_teams.size() - 1);
      for (int i = 0; i < n_new_teams_per_set; i++) {
         team* parent_team1;
         team* parent_team2;
         if (GetParam<int>("tournament_size") > 0) {  // Tournament selection
              parent_team1 = TeamSelector_Tournament(candidate_parent_teams);
              parent_team2 = TeamSelector_Tournament(candidate_parent_teams);
            } else {  // Random selection
              parent_team1 = candidate_parent_teams[disP(rngs_[TPG_SEED])];
              parent_team2 = candidate_parent_teams[disP(rngs_[TPG_SEED])];
            }
         team *child_team;
         // Maybe use crossover
         if (real_dist_(rngs_[TPG_SEED]) < GetParam<double>("pmx")) {
            child_team = TeamCrossover(parent_team1, parent_team2);
            AddAncestorToPhylogeny(parent_team1, child_team);
            AddAncestorToPhylogeny(parent_team2, child_team);
         }  else {
            child_team = CloneTeam(parent_team1);
            AddAncestorToPhylogeny(parent_team1, child_team);
         }
         AddTeamToPhylogeny(child_team);
         // Mutate child team
         ApplyVariationOps(child_team, new_teams_count);
         AddTeam(child_team);
         new_teams_count++;
      }
   }
   oss << "genTms t " << GetState("t_current") << " Msz " << team_pop_.size()
       << " Lsz " << program_pop_.size() << " mSz";
   for (size_t mem_t = 0; mem_t < MemoryEigen::kNumMemoryType_; mem_t++) {
      oss << " " << _Memory[mem_t].size();
   }
   oss << _Memory.size() << " eLSz "
       << _numEliteTeamsCurrent[GetState("phase")];
   oss << " nNTms " << new_teams_count << endl;
}

/******************************************************************************/
void TPG::ApplyVariationOps(team *team_to_modify, int &n_new_teams) {
   uniform_int_distribution<int> disL(0, program_pop_.size() - 1);
   // Mutate team
   TeamMutator_RemovePrograms(team_to_modify);
   TeamMutator_AddPrograms(team_to_modify);
   TeamMutator_ProgramOrder(team_to_modify);
   // Mutate programs
   // Clone before modifying ?
   if (real_dist_(rngs_[TPG_SEED]) < GetParam<double>("p_clone_program")) {
      set<RegisterMachine *, RegisterMachineIdComp> new_team_programs =
          team_to_modify->CopyMembers();
      for (auto prog : new_team_programs) {
         if (real_dist_(rngs_[TPG_SEED]) < 1.0 / new_team_programs.size()) {
            // TODO(skelly): add/remove changes order and thus behaviour?
            team_to_modify->RemoveProgram(prog);
            RegisterMachine *prog_clone = CloneProgram(prog);
            // ProgramMutator_Memory(prog_clone);
            ProgramMutator_Instructions(prog_clone);
            ProgramMutator_ActionPointer(prog_clone, team_to_modify,
                                         n_new_teams);
            team_to_modify->AddProgram(prog_clone);
            AddProgram(prog_clone);
         }
      }
   } else {  // Modify without cloning
      for (auto prog : team_to_modify->members_) {
         if (real_dist_(rngs_[TPG_SEED]) < 1.0 / team_to_modify->size()) {
            // ProgramMutator_Memory(prog);
            ProgramMutator_Instructions(prog);
            ProgramMutator_ActionPointer(prog, team_to_modify, n_new_teams);
         }
      }
   }
}

/******************************************************************************/
team* TPG::GetBestTeam() {
   set<team*, teamFitnessLexicalCompare> teams;
   std::copy_if(team_pop_.begin(), team_pop_.end(),
                std::inserter(teams, teams.end()),
                [](const team* tm) { return tm->root_; });
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
   phylo_graph_[tm->id_].fitness = tm->fit_;
   phylo_graph_[tm->id_].numActiveFeatures = tm->numActiveFeatures_;
   phylo_graph_[tm->id_].numActivePrograms = tm->numActivePrograms_;
   phylo_graph_[tm->id_].numActiveTeams = tm->numActiveTeams_;
   phylo_graph_[tm->id_].numEffectiveInstructions =
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
      for (auto tm : GetRootTeamsInVec()) {
         tm->elite(GetState("phase"), false);  // mark team as not elite
         if (tm->numOutcomes(GetState("phase"), task) >=
             tasks[task]->GetNumEval(GetState("phase"))) {
            tm->fit_ = tm->GetMedianOutcome(GetState("phase"), task, GetState("fitMode"));
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
   for (auto tm : GetRootTeamsInVec()) {
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
         auto raw_mean_score = tm->GetMedianOutcome(
             GetState("phase"), set[task], GetState("fitMode"));
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
               phylo_graph_[tm->id_].fitnessBin = tm->fitnessBin();
               phylo_graph_[tm->id_].fitness = tm->fit_;

               phylo_graph_[tm->id_].taskFitnesses.clear();
               for (int task = 0; task < GetState("n_task"); task++) {
                  phylo_graph_[tm->id_].taskFitnesses.push_back(tm->GetMedianOutcome(GetState("phase"), task, GetState("fitMode")));
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
   uniform_int_distribution<int> dis_team_size(
       1, GetParam<int>("max_initial_team_size"));
   int initial_team_size = dis_team_size(rngs_[TPG_SEED]);
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
      phyloRecord p;  // TODO(skelly): make pointer?
      phylo_graph_.insert(pair<long, phyloRecord>(new_team->id_, p));
      phylo_graph_[new_team->id_].gtime = 0;
   }
   oss << "InitTms Msz " << team_pop_.size() << " Lsz " << program_pop_.size() << " rSz "
       << GetRootTeamsInVec().size() << " mSz";
   for (size_t mem_t = 0; mem_t < MemoryEigen::kNumMemoryType_; mem_t++) {
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
void TPG::GetPolicyFeatures(int hostId, set<long> &features, bool active) {
   features.clear();
   set<team *, teamIdComp> visited_teams;
   if (hostId == -1) {
      for (auto tm : team_pop_) {
         if (tm->root_) {
            visited_teams.clear();
            tm->policyFeatures(_teamMap, visited_teams, features, active);
         }
      }
   } else {
      _teamMap[hostId]->policyFeatures(_teamMap, visited_teams, features,
                                       active);
   }
}

// /******************************************************************************/
// // Print graph defined by <rootTeam> in DOT format for GraphViz
// void TPG::printGraphDot(
//     team *rootTeam, size_t frame, int episode, int step, size_t depth,
//     vector<RegisterMachine *> allPrograms, vector<RegisterMachine *> winningPrograms,
//     vector<set<long>> decisionFeatures,
//     vector<set<MemoryEigen *, MemoryEigenIdComp>> decisionMemories,
//     vector<team *> teamPath, bool drawPath,
//     set<team *, teamIdComp> visitedTeamsAllTasks) {
//   // unused arguments
//   (void)decisionFeatures;
//   (void)decisionMemories;
//   (void)allPrograms;

//   // just use winning programs up to a specific graph depth
//   // vector<program*> winningProgramsDepth(winningPrograms.begin(),
//   // winningPrograms.begin()+depth);

//   vector<RegisterMachine *> winningProgramsDepth(winningPrograms.begin(),
//                                          winningPrograms.end());

//   double nodeWidth = 2.0;
//   double edgeWidth_1 = 1;  // 5;
//   double edgeWidth_2 = 30;
//   double arrowSize_1 = 1;  // 0.1;
//   double arrowSize_2 = 1;  // 0.2;

//   char outputFilename[80];
//   ofstream ofs;

//   set<team *, teamIdComp> teams;
//   set<RegisterMachine *, RegisterMachineIdComp> programs;
//   set<MemoryEigen *, MemoryEigenIdComp> memories;

//   //(void)visitedTeamsAllTasks;
//   for (auto it = visitedTeamsAllTasks.begin(); it !=
//   visitedTeamsAllTasks.end();
//        it++) {
//     set<RegisterMachine *, RegisterMachineIdComp> p = (*it)->CopyMembers();
//     programs.insert(p.begin(), p.end());
//   }
//   for (auto it = programs.begin(); it != programs.end(); it++) {
//     for (size_t mem_t = 0; mem_t < MemoryEigen::kNumMemoryType_; mem_t++) {
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
//   //    if ((*leiter)->action_ < 0)
//   //       ofs << " a_" << ((*leiter)->action_*-1)-1 << "_" <<
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

//   ////MemoryEigen registers
//   // for(auto meiter = memories.begin(); meiter != memories.end(); meiter++)
//   //    ofs << " m_" << (*meiter)->id_ << " [shape=invhouse, style=filled,
//   //    fillcolor=grey, label=\"\", regular=1, width=" << nodeWidth << "]" <<
//   //    endl;

//   // program -> team edges
//   for (auto leiter = programs.begin(); leiter != programs.end(); leiter++) {
//     if ((*leiter)->action_ >= 0 &&
//         find(visitedTeamsAllTasks.begin(), visitedTeamsAllTasks.end(),
//              _teamMap[(*leiter)->action_]) != visitedTeamsAllTasks.end()) {
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
//           << "t_" << (*leiter)->action_ << " [arrowsize=" << as
//           << ", penwidth=" << w << " color=" << col.c_str() << "];" << endl;
//     }
//   }

//   ////program -> MemoryEigen edges
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
//     list<RegisterMachine *> mem;
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
//   set<RegisterMachine *, RegisterMachineIdComp> programs;
//   set<MemoryEigen *, MemoryEigenIdComp> memories;

//   for (auto it = visitedTeamsAllTasks.begin(); it !=
//   visitedTeamsAllTasks.end();
//        it++) {
//     set<RegisterMachine *, RegisterMachineIdComp> p = (*it)->CopyMembers();  // no need o
//     copy programs.insert(p.begin(), p.end());
//   }
//   for (auto it = programs.begin(); it != programs.end(); it++) {
//     for (size_t mem_t = 0; mem_t < MemoryEigen::kNumMemoryType_; mem_t++) {
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
//   //    if ((*leiter)->action_ >= 0 && find(visitedTeamsAllTasks.begin(),
//   //    visitedTeamsAllTasks.end(), _teamMap[(*leiter)->action_]) !=
//   //    visitedTeamsAllTasks.end()){
//   //       ofs << " p_" << (*leiter)->id_ << "->" << "t_"<<
//   (*leiter)->action_
//   //       << " [arrowsize=" << arrowSize_1 << ", penwidth=" << "1" << "
//   color="
//   //       << "black" << "];" << endl;
//   //    }
//   // }

//   ////team -> program edges
//   // for(auto teiter = visitedTeamsAllTasks.begin(); teiter !=
//   // visitedTeamsAllTasks.end(); teiter++){
//   //    list < RegisterMachine * > mem;
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
//     list<RegisterMachine *> mem;
//     (*teiter)->members(&mem);
//     for (auto leiter = mem.begin(); leiter != mem.end(); leiter++) {
//       // ofs << " t_" << (*teiter)->id_ << "->p_" << (*leiter)->id_ << "
//       // [arrowsize=" << arrowSize_1  << ", penwidth=" << "1" << " color=" <<
//       // "black" << "];" << endl;
//       if ((*leiter)->action_ >= 0 &&
//           find(visitedTeamsAllTasks.begin(), visitedTeamsAllTasks.end(),
//                _teamMap[(*leiter)->action_]) != visitedTeamsAllTasks.end())
//                {
//         ofs << " t_" << (*teiter)->id_ << "->t_" << (*leiter)->action_
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
         if (prog->action_ >= 0 &&
             find(visitedTeamsAllTasks.begin(), visitedTeamsAllTasks.end(),
                  _teamMap[prog->action_]) != visitedTeamsAllTasks.end()) {
            ofs << " t_" << tm->id_ << "->t_" << prog->action_
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
//     vector<RegisterMachine *> allPrograms, vector<RegisterMachine *> winningPrograms,
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
//   set<RegisterMachine *, RegisterMachineIdComp> programs;
//   set<MemoryEigen *, MemoryEigenIdComp> memories;
//   vector<RegisterMachine *> winningProgramsDepth(winningPrograms.begin(),
//                                          winningPrograms.end());

//   for (auto it = visitedTeamsAllTasks.begin(); it !=
//   visitedTeamsAllTasks.end();
//        it++) {
//     set<RegisterMachine *, RegisterMachineIdComp> p = (*it)->CopyMembers();  // no need to
//     copy programs.insert(p.begin(), p.end());
//   }
//   for (auto it = programs.begin(); it != programs.end(); it++) {
//     for (size_t mem_t = 0; mem_t < MemoryEigen::kNumMemoryType_; mem_t++) {
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
//   //    list < RegisterMachine * > mem;
//   //    (*teiter)->members(&mem);
//   //    for(auto leiter = mem.begin(); leiter != mem.end(); leiter++){
//   //       if ((*leiter)->action_ >= 0 && find(visitedTeamsAllTasks.begin(),
//   //       visitedTeamsAllTasks.end(), _teamMap[(*leiter)->action_]) !=
//   //       visitedTeamsAllTasks.end()){
//   //          ofs << " t_" << (*teiter)->id_ << "->t_" << (*leiter)->action_
//   //          << " [arrowsize=" << arrowSize_1  << ", penwidth=" << "1" << "
//   //          color=" << "black" << "];" << endl;
//   //       }
//   //    }
//   // }

//   ////team -> team edges path
//   // if (step > 0){
//   //    for(size_t t = 0; t < teamPath.size()-1; t++){
//   //       list < RegisterMachine * > mem;
//   //       teamPath[t]->members(&mem);
//   //       for(auto leiter = mem.begin(); leiter != mem.end(); leiter++){
//   //          if ((*leiter)->action_ >= 0 && teamPath[t+1]->id_ ==
//   //          _teamMap[(*leiter)->action_]->id_){
//   //             ofs << " t_" << teamPath[t]->id_ << "->t_" <<
//   //             (*leiter)->action_ << " [arrowsize=" << arrowSize_2  << ",
//   //             penwidth=" << edgeWidth_2 << " color=" << "black" << "];" <<
//   //             endl;
//   //          }
//   //       }
//   //    }
//   // }

//   // program -> team edges
//   for (auto leiter = programs.begin(); leiter != programs.end(); leiter++) {
//     if ((*leiter)->action_ >= 0 &&
//         find(visitedTeamsAllTasks.begin(), visitedTeamsAllTasks.end(),
//              _teamMap[(*leiter)->action_]) != visitedTeamsAllTasks.end()) {
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
//           << "t_" << (*leiter)->action_ << " [arrowsize=" << as
//           << ", penwidth=" << w << " color=" << col.c_str() << "];" << endl;
//     }
//   }

//   // team -> program edges
//   for (auto teiter = visitedTeamsAllTasks.begin();
//        teiter != visitedTeamsAllTasks.end(); teiter++) {
//     list<RegisterMachine *> mem;
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

      for (long ancId : phylo_graph_[currId].ancestorIds) {
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
      double fitness = phylo_graph_[id].fitness;
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
   team *bestTeam = *(team_pop_.begin());
   if (singleBest && teamId == -1) bestTeam = GetBestTeam();
   ostringstream tmposs;
   map<point *, double, pointLexicalLessThan> allOutcomes;
   // map < point *, double > :: iterator myoiter;
   vector<int> behaviourSequence;
   set<team *, teamIdComp> visitedTeams;

   for (auto teiter = team_pop_.begin(); teiter != team_pop_.end(); teiter++) {
      if ((!singleBest && (*teiter)->root() &&
           teamId == -1) ||                             // all root teams
          (!singleBest && (*teiter)->id_ == teamId) ||  // specific team
          (singleBest &&
           (*teiter)->id_ == bestTeam->id_))  // singleBest root team
      {
         // dispatching that the best fitness score has been triggered
         EventDispatcher::instance().notify(
            EventType::SELECTION, {
               {"generation", std::to_string(t)},
               {"best_fitness", std::to_string((*teiter)->GetMedianOutcome(0, 0, 0))}, // best fitness score occurs on p0t0a0 
               {"team_id", std::to_string((*teiter)->id_)}
            }
         );          
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
                     oss << (*teiter)->GetMedianOutcome(phs, task, i);                    
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
         oss << " mnProgIns " << VectorMean<int>(programInstructionCounts);

         oss << " ePIns "
             << accumulate(effectiveProgramInstructionCounts.begin(),
                           effectiveProgramInstructionCounts.end(), 0);
         oss << " mnEProgIns " << VectorMean<int>(effectiveProgramInstructionCounts);
         set<RegisterMachine *, RegisterMachineIdComp> programs;
         // set<MemoryEigen *, MemoryEigenIdComp> memories;
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
            op_countsSingle = (*it)->op_counts_;
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
         oss << " mnTmSzR " << VectorMean<int>(tmSizesRoot) << " mnTmSzS "
             << (tmSizesSub.size() > 0 ? VectorMean<int>(tmSizesSub) : 0);

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

   team *bestTeam = *(team_pop_.begin());
   if (singleBest && teamId == -1) bestTeam = GetBestTeam();
   ostringstream tmposs;
   map<point *, double, pointLexicalLessThan> allOutcomes;
   // map < point *, double > :: iterator myoiter;
   vector<int> behaviourSequence;
   set<team *, teamIdComp> visitedTeams;
   for (auto teiter = team_pop_.begin(); teiter != team_pop_.end(); teiter++) {
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
         double mnProgIns = VectorMean<int>(programInstructionCounts);
         int ePIns = accumulate(effectiveProgramInstructionCounts.begin(),
                                effectiveProgramInstructionCounts.end(), 0);
         double mnEProgIns = VectorMean<int>(effectiveProgramInstructionCounts);
         api_client_->LogMetric("teamInfo/pIns", std::to_string(pIns), "", gen);
         api_client_->LogMetric("teamInfo/mnProgIns", std::to_string(mnProgIns),
                                "", gen);
         api_client_->LogMetric("teamInfo/ePIns", std::to_string(ePIns), "",
                                gen);
         api_client_->LogMetric("teamInfo/mnEProgIns",
                                std::to_string(mnEProgIns), "", gen);

         set<RegisterMachine *, RegisterMachineIdComp> programs;
         // set<MemoryEigen *, MemoryEigenIdComp> memories;
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
            op_countsSingle = (*it)->op_counts_;
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

         double mnTmSzR = VectorMean<int>(tmSizesRoot);
         int mnTmSzS = (tmSizesSub.size() > 0 ? VectorMean<int>(tmSizesSub) : 0);
         api_client_->LogMetric("teamInfo/mnTmSzR", std::to_string(mnTmSzR), "",
                                gen);
         api_client_->LogMetric("teamInfo/mnTmSzS", std::to_string(mnTmSzS), "",
                                gen);
      }
   }
}

/******************************************************************************/
void TPG::RegisterMachineCrossover(RegisterMachine* p1, RegisterMachine* p2,
                                   RegisterMachine** c1, RegisterMachine** c2) {
   int split1, split2;

   // Split parent 1 (p1) into 3 random chunks
   std::vector<std::vector<instruction*>> p1_chunks(3);
   std::uniform_int_distribution<> dis1(0, p1->instructions_.size() - 1);
   split1 = dis1(rngs_[TPG_SEED]);
   split2 = dis1(rngs_[TPG_SEED]);
   while (split2 == split1) {
      split2 = dis1(rngs_[TPG_SEED]);
   }
   if (split1 > split2) {
      std::swap(split1, split2);
   }
   p1_chunks[0] = std::vector<instruction*>(p1->instructions_.begin(),
                                            p1->instructions_.begin() + split1);
   p1_chunks[1] = std::vector<instruction*>(p1->instructions_.begin() + split1,
                                            p1->instructions_.begin() + split2);
   p1_chunks[2] = std::vector<instruction*>(p1->instructions_.begin() + split2,
                                            p1->instructions_.end());

   // Split parent 2 (p2) into 3 random chunks
   std::vector<std::vector<instruction*>> p2_chunks(3);
   std::uniform_int_distribution<> dis2(0, p2->instructions_.size() - 1);
   split1 = dis2(rngs_[TPG_SEED]);
   split2 = dis2(rngs_[TPG_SEED]);
   while (split2 == split1) {
      split2 = dis2(rngs_[TPG_SEED]);
   }
   if (split1 > split2) {
      std::swap(split1, split2);
   }
   p2_chunks[0] = std::vector<instruction*>(p2->instructions_.begin(),
                                            p2->instructions_.begin() + split1);
   p2_chunks[1] = std::vector<instruction*>(p2->instructions_.begin() + split1,
                                            p2->instructions_.begin() + split2);
   p2_chunks[2] = std::vector<instruction*>(p2->instructions_.begin() + split2,
                                            p2->instructions_.end());

   // Cretae child 1 (c1) from parent chunks {p1-0, p2-1, p1-2}
   std::vector<instruction*> c1_instructions;
   c1_instructions.insert(c1_instructions.end(), p1_chunks[0].begin(),
                          p1_chunks[0].end());
   c1_instructions.insert(c1_instructions.end(), p2_chunks[1].begin(),
                          p2_chunks[1].end());
   c1_instructions.insert(c1_instructions.end(), p1_chunks[2].begin(),
                          p1_chunks[2].end());
   *c1 = new RegisterMachine(p1->action_, c1_instructions, params_, state_,
                             rngs_[TPG_SEED], _ops);

   // Cretae child 2 (c2) from parent chunks {p2-0, p1-1, p2-2}
   std::vector<instruction*> c2_instructions;
   c2_instructions.insert(c2_instructions.end(), p2_chunks[0].begin(),
                          p2_chunks[0].end());
   c2_instructions.insert(c2_instructions.end(), p1_chunks[1].begin(),
                          p1_chunks[1].end());
   c2_instructions.insert(c2_instructions.end(), p2_chunks[2].begin(),
                          p2_chunks[2].end());
   *c2 = new RegisterMachine(p2->action_, c2_instructions, params_, state_,
                             rngs_[TPG_SEED], _ops);                         
}

// /******************************************************************************/
// // Algorithm 5.1 (linear crossover)
//    void TPG::LinearCrossover(RegisterMachine *gp1, 
//                                    RegisterMachine *gp2,
//                                    RegisterMachine **c1, 
//                                    RegisterMachine **c2) {
//    int dcMax = min(gp1->instructions_.size(), gp2->instructions_.size());
//    int dsMax = dcMax;
//    int lsMax = dcMax;
//    int pos1, pos2;

//    vector<RegisterMachine *> parents{gp1, gp2};
//    vector<int> segLengths{1, 1};

//    if (gp1->instructions_.size() > gp2->instructions_.size()) swap(parents[0], parents[1]);

//    // 1
//    uniform_int_distribution<> dis1(0, parents[0]->instructions_.size() - 1);
//    pos1 = dis1(rngs_[TPG_SEED]);
//    uniform_int_distribution<> dis2(0, parents[1]->instructions_.size() - 1);
//    do {
//       pos2 = dis2(rngs_[TPG_SEED]);
//    } while (abs(pos1 - pos2) > min(static_cast<int>(parents[0]->instructions_.size()) - 1, dcMax));

//    // 2,3
//    uniform_int_distribution<> dis3(1, min(static_cast<int>(parents[0]->instructions_.size()) - pos1, lsMax));
//    segLengths[0] = dis3(rngs_[TPG_SEED]);
//    uniform_int_distribution<> dis4(1, min(static_cast<int>(parents[1]->instructions_.size()) - pos2, lsMax));
//    do {
//       segLengths[1] = dis4(rngs_[TPG_SEED]);
//    } while (abs(segLengths[0] - segLengths[1]) > dsMax);

//    // 4
//    if (segLengths[0] > segLengths[1]) swap(segLengths[0], segLengths[1]);

//    // 5
//    if (static_cast<int>(gp1->instructions_.size()) - (segLengths[1] - segLengths[0]) < 1 ||
//        static_cast<int>(gp2->instructions_.size()) + (segLengths[1] - segLengths[0]) >
//            GetParam<int>("max_prog_size")) {
//       if (real_dist_(rngs_[TPG_SEED]) < 0.5)
//          segLengths[1] = segLengths[0];
//       else
//          segLengths[0] = segLengths[1];

//       if (pos1 + segLengths[0] > static_cast<int>(gp1->instructions_.size()))
//          segLengths[0] = segLengths[1] = gp1->instructions_.size() - pos1;
//    }

//    vector<instruction *> parentProg1 = gp1->instructions_;
//    vector<instruction *> parentProg2 = gp2->instructions_;

//    vector<instruction *> childProg1 = gp1->instructions_;
//    vector<instruction *> childProg2 = gp2->instructions_;

//    // // exchange seg1 in gp1 by seg2 in gp2
//    // childProg1.clear();
//    // auto start = parentProg1.begin();
//    // auto end = parentProg1.begin() + pos1;
//    // std::copy(start, end, back_inserter(childProg1));
//    // start = parentProg2.begin() + pos2;
//    // end = parentProg2.begin() + pos2 + segLengths[1];
//    // std::copy(start, end, back_inserter(childProg1));
//    // start = parentProg1.begin() + pos1 + segLengths[0];
//    // end = parentProg1.end();
//    // std::copy(start, end, back_inserter(childProg1));

//    // cerr << "dbg sz " << childProg1.size() << ":";
//    // for (auto i : childProg1) {
//    //    cerr << " " << i->memIndices_;
//    // }

//    // // exchange seg2 in gp2 by seg1 in gp1
//    // childProg2.clear();
//    // start = parentProg2.begin();
//    // end = parentProg2.begin() + pos2;
//    // copy(start, end, back_inserter(childProg2));
//    // start = parentProg1.begin() + pos1;
//    // end = parentProg1.begin() + pos1 + segLengths[0];
//    // copy(start, end, back_inserter(childProg2));
//    // start = parentProg2.begin() + pos2 + segLengths[1];
//    // end = parentProg2.end();
//    // copy(start, end, back_inserter(childProg2));

//    *c1 = new RegisterMachine(gp1->action_, childProg1, params_, state_, rngs_[TPG_SEED], _ops);
//    *c2 = new RegisterMachine(gp2->action_, childProg2, params_, state_, rngs_[TPG_SEED], _ops);
// }

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
   long memberId = 0;
   long max_teamCount = -1;
   long max_programCount = -1;
   // long max_memoryCount = -1;
   int f;

   while (getline(iss, oneline)) {
      if (oneline.size() == 0) continue;
      auto outcome_fields = SplitString(oneline, ':');

      if (outcome_fields[0].compare("t") == 0)
         state_["t_current"] = atoi(outcome_fields[1].c_str());
      else if (outcome_fields[0].compare("active_task") == 0)
         state_["active_task"] = atoi(outcome_fields[1].c_str());
      else if (outcome_fields[0].compare("fitMode") == 0)
         params_["fit_mode"] = atoi(outcome_fields[1].c_str());
      else if (outcome_fields[0].compare("phase") == 0)
         state_["phase"] = atoi(outcome_fields[1].c_str());
      else if (outcome_fields[0].compare("MemoryEigen") == 0) {
         // max_memoryCount =
            //  std::max(max_memoryCount, atol(outcome_fields[1].c_str()));
            long prog_id = atol(outcome_fields[1].c_str());
         AddMemory(prog_id, new MemoryEigen(outcome_fields));
      } else if (outcome_fields[0].compare("RegisterMachine") == 0) {
         max_programCount =
             std::max(max_programCount, atol(outcome_fields[1].c_str()));
         AddProgram(new RegisterMachine(outcome_fields, _Memory, params_,
                                        rngs_[TPG_SEED]));
      } else if (outcome_fields[0].compare("team") == 0) {
         team *m;
         f = 1;
         long id = atoi(outcome_fields[f++].c_str());
         if (id > max_teamCount) max_teamCount = id;
         long gtime = atoi(outcome_fields[f++].c_str());
         m = new team(gtime, id);
         m->_n_eval = atoi(outcome_fields[f++].c_str());
         // add programs in order
         for (size_t ii = f; ii < outcome_fields.size(); ii++) {
            memberId = atoi(outcome_fields[ii].c_str());
            m->AddProgram(program_pop_[memberId]);
         }
         AddTeam(m);
      } else if (outcome_fields[0].compare("incoming_progs") == 0) {
         f = 1;
         long id = atoi(outcome_fields[f++].c_str());
         for (size_t ii = f; ii < outcome_fields.size(); ii++) {
            long incomingId = atoi(outcome_fields[ii].c_str());
            _teamMap[id]->AddIncomingProgram(incomingId);
         }
      } else if (!fromString && outcome_fields[0].compare("phyloNode") == 0 &&
                 find(outcome_fields.begin(), outcome_fields.end(), "gtime") ==
                     outcome_fields.end()) {
         f = 1;
         long id = atoi(outcome_fields[f++].c_str());
         phylo_graph_.insert(pair<long, phyloRecord>(id, phyloRecord()));
         phylo_graph_[id].gtime = atoi(outcome_fields[f++].c_str());
         phylo_graph_[id].dtime = atoi(outcome_fields[f++].c_str());
         phylo_graph_[id].fitnessBin = atoi(outcome_fields[f++].c_str());
         phylo_graph_[id].fitness = atof(outcome_fields[f++].c_str());
         phylo_graph_[id].root =
             atoi(outcome_fields[f++].c_str()) > 0 ? true : false;
      } else if (!fromString && outcome_fields[0].compare("phyloLink") == 0 &&
                 find(outcome_fields.begin(), outcome_fields.end(), "from") ==
                     outcome_fields.end())
         phylo_graph_[atoi(outcome_fields[1].c_str())].adj.push_back(
             atoi(outcome_fields[2].c_str()));
      else if (!fromString && outcome_fields[0].compare("ancestorIds") == 0) {
         f = 1;
         long id = atoi(outcome_fields[f++].c_str());
         for (size_t ii = f; ii < outcome_fields.size(); ii++) {
            long aid = atoi(outcome_fields[ii].c_str());
            phylo_graph_[id].ancestorIds.insert(aid);
         }
      }
   }
   state_["program_count"] = max_programCount + 1;
   state_["team_count"] = max_teamCount + 1;
   // state_["memory_count"] = max_memoryCount + 1;
}

/******************************************************************************/
void TPG::recalculateProgramRefs() {
   for (auto p : program_pop_) p.second->nrefs_ = 0;
   for (auto tm : team_pop_)
      for (auto p : tm->members_) p->nrefs_++;
}

/******************************************************************************/
void TPG::SanityCheck() { TeamSizesMatchProgRefs(); }

/******************************************************************************/
void TPG::TeamSizesMatchProgRefs() {
   int sum_team_sizes = 0;
   int sum_prog_refs = 0;
   for (auto tm : team_pop_) sum_team_sizes += tm->size();
   for (auto prog : program_pop_) sum_prog_refs += prog.second->nrefs_;
   if (sum_prog_refs != sum_team_sizes) {
      std::string error_message =
          "Program reference mismatch. sum_team_sizes " +
          to_string(sum_team_sizes) + " sum_prog_refs " +
          to_string(sum_prog_refs);
      die(__FILE__, __FUNCTION__, __LINE__, error_message.c_str());
   }
}

/******************************************************************************/
// TODO(skelly): survivor selection
void TPG::SelectTeams() {

   int n_old_deleted = 0;
   int n_deleted = 0;
   int n_root_remaining = 0;

   for (auto tm : GetRootTeamsInVec()) {
      if (!tm->elite(GetState("phase")) && !isElitePS(tm, GetState("phase"))) {
         phylo_graph_[tm->id_].dtime = GetState("t_current");
         n_deleted++;
         n_old_deleted += tm->gtime_ < GetState("t_current") ? 1 : 0;
         RemoveTeam(tm);
      }
      n_root_remaining++;
   }

   CleanupProgramsWithNoRefs();

   oss << "selTms t " << GetState("t_current") << " Msz " << team_pop_.size()
       << " Lsz " << program_pop_.size() << " mrSz " << n_root_remaining << " mSz";
   for (size_t mem_t = 0; mem_t < MemoryEigen::kNumMemoryType_; mem_t++) {
      oss << " " << _Memory[mem_t].size();
   }
   oss << " eLSz " << _numEliteTeamsCurrent[GetState("phase")] << " nDel "
       << n_deleted << " nOldDel " << n_old_deleted << " nOldDelPr "
       << (double)n_old_deleted / n_deleted;
   oss << endl;
}

/******************************************************************************/
void TPG::CleanupProgramsWithNoRefs() {
   vector<long> deletedIds;
   for (auto it = program_pop_.begin(); it != program_pop_.end();) {
      auto prog = it->second;
      if (prog->nrefs_ < 1) {
         if (prog->action_ >= 0) {
            if (_teamMap[prog->action_]->inDeg() == 1) {
               phylo_graph_[_teamMap[prog->action_]->id_].root = true;
            }
            _teamMap[prog->action_]->removeIncomingProgram(prog->id_);
            // If team was a subsumed root clone that has now become a root
            // itself, just delete it
            if (_teamMap[prog->action_]->root() &&
                _teamMap[prog->action_]->cloneId_ != -1) {
               auto it = _teamMap.find(_teamMap[prog->action_]->cloneId_);
               if (it != _teamMap.end())
                  it->second->clones_ = it->second->clones_ - 1;
               team* tm = _teamMap[prog->action_];
               phylo_graph_[tm->id_].dtime = GetState("t_current");
               RemoveTeam(tm);
            }
         }
         it = program_pop_.erase(it);
         delete prog;
      } else {
         it++;
      }
   }
}

// /******************************************************************************/
// void TPG::teamTaskRank(int phase, const vector<int> &objectives) {
//    oss << "TPG::teamTaskRank <team:avgRank>";
//    for (auto teiterA = team_pop_.begin(); teiterA != team_pop_.end(); teiterA++) {
//       if (!(*teiterA)->root()) continue;
//       vector<int> ranks;
//       for (size_t i = 0; i < objectives.size(); i++) ranks.push_back(1);
//       for (size_t o = 0; o < objectives.size(); o++) {
//          for (auto teiterB = team_pop_.begin(); teiterB != team_pop_.end(); teiterB++) {
//             if (!(*teiterB)->root()) continue;
//             if ((*teiterB)->getMeanOutcome(phase, objectives[o],
//                                            GetParam<int>("fit_mode"), false,
//                                            false) >
//                 (*teiterA)->getMeanOutcome(phase, objectives[o],
//                                            GetParam<int>("fit_mode"), false,
//                                            false))
//                ranks[o]++;
//          }
//       }

//       double rankSum = 0;
//       for (size_t i = 0; i < ranks.size(); i++) rankSum += ranks[i];
//       (*teiterA)->fit_ = 1 / (rankSum / ranks.size());
//       oss << " " << (*teiterA)->id_ << ":" << (*teiterA)->fit_;
//    }
//    oss << endl;
// }

/******************************************************************************/
void TPG::updateMODESFilters(bool roots) {
   (void)roots;
   vector<long> symbiontIntersection;
   symbiontIntersection.reserve(100);
   vector<long> symbiontUnion;
   symbiontUnion.reserve(100);

   if (GetState("t_current") != GetState("t_start")) {
      _persistenceFilterA.clear();
      for (auto tm : GetRootTeamsInVec()) {   
         vector<long> ancestorIds;
         tm->getAncestorIds(ancestorIds);
         for (auto pr = _allComponentsA.begin(); pr != _allComponentsA.end();
              pr++)
            if (find(ancestorIds.begin(), ancestorIds.end(), (*pr).first) !=
                    ancestorIds.end() &&
                tm->hasOutcome(0, _TRAIN_PHASE, 0)) {
               // found an ancestor of *teiter in _allComponentsA, add
               // *teiter to _persistenceFilterA
               _persistenceFilterA.insert(
                   pair<long, modesRecord>(tm->id_, modesRecord()));
               // store active programs for novelty metric
               set<team *, teamIdComp> teams;
               set<RegisterMachine *, RegisterMachineIdComp> programs;
               tm->GetAllNodes(_teamMap, teams, programs);
               for (auto leiter = programs.begin(); leiter != programs.end();
                    leiter++) {
                  _persistenceFilterA[tm->id_].activeProgramIds.insert(
                      (*leiter)->id_);
                  _persistenceFilterA[tm->id_]
                      .effectiveInstructionsTotal += static_cast<int>((*leiter)->instructions_effective_.size());
               }
               for (auto tm2 : teams)
                  _persistenceFilterA[tm->id_].activeTeamIds.insert(tm2->id_);
               _persistenceFilterA[tm->id_].runTimeComplexityIns =
                   tm->runTimeComplexityIns();
               _persistenceFilterA[tm->id_].behaviourString =
                   tm->getBehaviourString(0, _TRAIN_PHASE);
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
   for (auto tm : GetRootTeamsInVec()) {
      _allComponentsA.insert(pair<long, modesRecord>(tm->id_, modesRecord()));
   }
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
      set<MemoryEigen *> memories;
      set<RegisterMachine *, RegisterMachineIdComp> programs;
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
      for (auto prog : programs) {
         ofs << prog->ToStringMemory();
      }
      for (auto prog : programs) {
         ofs << prog->ToString(GetParam<int>("skip_introns"));
      }
      for (auto tm : teamsAll) {
         ofs << tm->checkpoint();
      }
   } else {  // Include all memories, teams, and programs
      for (auto key : program_pop_) {
         ofs << key.second->ToStringMemory();
      }
      for (auto key : program_pop_) {
         ofs << key.second->ToString(false);  // Write all instructions
      }
      for (auto tm : team_pop_) {
         ofs << tm->checkpoint();
      }
      ofs << PhylogenyToString();
   }
   ofs << "end" << endl;
   ofs.close();
}

/******************************************************************************/
std::string TPG::PhylogenyToString() {
   std::stringstream ss;
   ss << "phyloNode:id:gtime:dtime:fitness:root" << endl;
   ss << "phyloLink:from,to" << endl;
   for (auto it = phylo_graph_.begin(); it != phylo_graph_.end(); it++) {
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
   set<MemoryEigen *> memories;
   set<RegisterMachine *, RegisterMachineIdComp> programs;
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
   // for (auto mem : memories) {
   //    ss << mem->ToString();
   // }
   for (auto prog : programs) {
      ss << prog->ToStringMemory();
   }
   for (auto prog : programs) {
      ss << prog->ToString(GetParam<int>("skip_introns"));
   }
   for (auto tm : teams) {
      ss << tm->checkpoint();
   }
   ss << endl;
   s = ss.str();
}