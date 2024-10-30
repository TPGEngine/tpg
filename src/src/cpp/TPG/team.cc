#include "team.h"

#include <algorithm>
#include <limits>

// Allow duplicates
void team::AddProgram(program *prog, int position) {
    auto it = members_.begin();
    advance(it, position);
    members_.insert(it, prog);
    if (prog->action() < 0) n_atomic_++;
    members_run_.resize(members_.size());  // put in mark introns
    prog->nrefs_++;
}

/******************************************************************************/
string team::checkpoint() const {
    ostringstream oss;
    oss << "team:" << id_ << ":" << gtime_ << ":" << _n_eval;
    for (auto prog : members_) {
        oss << ":" << prog->id_;
    }
    oss << endl;
    if (incomingPrograms_.size() > 0) {
        oss << "incoming_progs:" << id_;
        for (auto &ip : incomingPrograms_) {
            oss << ":" << ip;
        }
        oss << endl;
    }
    return oss.str();
}

/******************************************************************************/
void team::InitMemory(map<long, team *> &teamMap,
                      std::unordered_map<std::string, std::any> &params) {
    set<team *, teamIdComp> teams;
    set<program *, programIdComp> programs;
    GetAllNodes(teamMap, teams, programs);
    for (auto prog : programs) {
        if (!isEqual(std::any_cast<double>(params["p_bid_mu_const"]), 0.0)) {
            prog->use_evolved_const_ = true;
            // Initialize working memory with evolved constants
            prog->CopyPrivateConstToWorking();
            // prog->ClearWorking();
        } else {
            // Initialize working memory with zeros
            prog->use_evolved_const_ = false;
            prog->ClearWorking();
        }
    }
}

/******************************************************************************/
void team::clone(map<long, phyloRecord> &phyloGraph, team **tm) {
    phyloGraph[(*tm)->id_].ancestorIds.insert(id_);
    for (auto prog : members_) {
        (*tm)->AddProgram(prog);
    }
    (*tm)->fitnessBins(fitnessBins_);
    (*tm)->cloneId_ = id_;
    clones_++;
}

/******************************************************************************/
void team::features(set<long> &F) const {
    if (F.empty() == false)
        die(__FILE__, __FUNCTION__, __LINE__, "feature set not empty");

    for (auto prog : members_) prog->features(F);
}

/******************************************************************************/
double team::novelty(int type, int kNN) const {
    multiset<double>::iterator it;
    double nov = 0;
    int i = 0;
    if (type == 0) {
        for (it = distances_0_.begin(); it != distances_0_.end() && i <= kNN;
             ++it, i++)
            nov += *it;
        return nov / i;
    } else if (type == 1) {
        for (it = distances_1_.begin(); it != distances_1_.end() && i <= kNN;
             ++it, i++)
            nov += *it;
        return nov / i;
    } else if (type == 2) {
        for (it = distances_2_.begin(); it != distances_2_.end() && i <= kNN;
             ++it, i++)
            nov += *it;
        return nov / i;
    } else
        return -1;
}

/******************************************************************************/
void team::updateComplexityRecord(map<long, team *> &teamMap, int rtcIndex) {
    (void)teamMap;
    // set <team *, teamIdComp> teams;
    // set <program *, programIdComp> programs;
    // set <memoryEigen *, memoryEigenIdComp> memories;
    // GetAllNodes(teamMap, teams, programs, memories, false);//not just
    // active programs _numActiveTeams = teams.size(); _numActivePrograms =
    // programs.size(); _numEffectiveInstructions = 0; _numActiveFeatures =
    // 0; for (auto leiter = programs.begin(); leiter != programs.end();
    // leiter++){
    //    _numEffectiveInstructions += (*leiter)->SizeEffective();
    //    _numActiveFeatures += (*leiter)->numFeatures();
    // }
    runTimeComplexityIns_ =
        getMeanOutcome(_TRAIN_PHASE, 0, rtcIndex, false, true);
    runTimeComplexityTms_ =
        getMeanOutcome(_TRAIN_PHASE, 0, rtcIndex - 1, false, true);
}

/******************************************************************************/
void team::updateComplexityRecord(map<long, team *> &teamMap, int rtcIndex,
                                  int auxInt, long auxIntMatch, int phase) {
    (void)teamMap;
    // set <team *, teamIdComp> teams;
    // set <program *, programIdComp> programs;
    // set <memoryEigen *, memoryEigenIdComp> memories;
    // GetAllNodes(teamMap, teams, programs, memories, false);//not just
    // active programs _numActiveTeams = teams.size(); _numActivePrograms =
    // programs.size(); _numEffectiveInstructions = 0; _numActiveFeatures =
    // 0; for (auto leiter = programs.begin(); leiter != programs.end();
    // leiter++){
    //    _numEffectiveInstructions += (*leiter)->SizeEffective();
    //    _numActiveFeatures += (*leiter)->numFeatures();
    // }
    runTimeComplexityIns_ =
        getMeanOutcome(phase, 0, rtcIndex, auxInt, auxIntMatch, false, true);
    runTimeComplexityTms_ = getMeanOutcome(phase, 0, rtcIndex - 1, auxInt,
                                           auxIntMatch, false, true);
}

// TODO(skelly): remove shared memory code
/******************************************************************************/
// void team::GetAllMemories(
//     map<long, team *> &teamMap, set<team *, teamIdComp> &visitedTeams,
//     set<memoryEigen *, memoryEigenIdComp> &memories) const {
//   visitedTeams.insert(teamMap[id_]);

//   for (auto prog : members_) {
//     for (int mem_t = 0; mem_t < memoryEigen::NUM_MEMORY_TYPES; mem_t++) {
//       memories.insert(prog->MemGet(mem_t));
//     }
//     if (prog->action() >= 0 &&
//         find(visitedTeams.begin(), visitedTeams.end(),
//              teamMap[prog->action()]) == visitedTeams.end())
//       teamMap[prog->action()]->GetAllMemories(teamMap, visitedTeams,
//       memories);
//   }
// }

/******************************************************************************/
// this version returns partial graph up to team tm
void team::GetAllNodes(map<long, team *> &teamMap,
                       set<team *, teamIdComp> &visitedTeams, long stopId,
                       bool skipRoot) const {
    if (!skipRoot || !root_) visitedTeams.insert(teamMap[id_]);
    for (auto prog : members_) {
        if (prog->action() >= 0 &&
            find(visitedTeams.begin(), visitedTeams.end(),
                 teamMap[prog->action()]) == visitedTeams.end() &&
            prog->action() != stopId)
            teamMap[prog->action()]->GetAllNodes(teamMap, visitedTeams, stopId,
                                                 skipRoot);
    }
}

/******************************************************************************/
void team::GetAllNodes(map<long, team *> &teamMap,
                       set<team *, teamIdComp> &visitedTeams,
                       set<program *, programIdComp> &programs) const {
    visitedTeams.insert(teamMap[id_]);
    for (auto prog : members_) {
        programs.insert(prog);
        if ((prog)->action() >= 0 &&
            find(visitedTeams.begin(), visitedTeams.end(),
                 teamMap[(prog)->action()]) == visitedTeams.end())
            teamMap[(prog)->action()]->GetAllNodes(teamMap, visitedTeams,
                                                   programs);
    }
}

/******************************************************************************/
void team::GetAllNodes(map<long, team *> &teamMap,
                       set<team *, teamIdComp> &visitedTeams,
                       set<program *, programIdComp> &programs,
                       set<memoryEigen *, memoryEigenIdComp> &memories)
                       const {
  visitedTeams.insert(teamMap[id_]);
  for (auto prog : members_) {
    programs.insert(prog);
    for (auto m : prog->privateMemory_) {
        memories.insert(m);
    }
    if (prog->action() >= 0 &&
        find(visitedTeams.begin(), visitedTeams.end(),
             teamMap[prog->action()]) == visitedTeams.end())
      teamMap[prog->action()]->GetAllNodes(teamMap, visitedTeams, programs, memories);
  }
}

/******************************************************************************/
void team::updatePolicyRoot(map<long, team *> &teamMap,
                            set<team *, teamIdComp> &visitedTeams,
                            long &rootId) {
    visitedTeams.insert(teamMap[id_]);
    addPolicyRootId(rootId);  // add even if this is the root

    for (auto prog : members_) {
        if (prog->action() >= 0 &&
            find(visitedTeams.begin(), visitedTeams.end(),
                 teamMap[prog->action()]) == visitedTeams.end())
            teamMap[prog->action()]->updatePolicyRoot(teamMap, visitedTeams,
                                                      rootId);
    }
}

/******************************************************************************/
// Fill F with every feature indexed by every program in this policy
// (tree).If we ever build massive policy tress, this should be changed to a
// more efficient traversal. For now just look at every node.
int team::policyFeatures(map<long, team *> &teamMap,
                         set<team *, teamIdComp> &visitedTeams, set<long> &F,
                         bool active) const {
    visitedTeams.insert(teamMap[id_]);

    set<long> featuresSingle;
    int numProgramsInPolicy = 0;
    for (auto prog : members_) {
        numProgramsInPolicy++;
        prog->features(featuresSingle);
        F.insert(featuresSingle.begin(), featuresSingle.end());
        if (prog->action() >= 0 &&
            find(visitedTeams.begin(), visitedTeams.end(),
                 teamMap[prog->action()]) == visitedTeams.end())
            numProgramsInPolicy += teamMap[prog->action()]->policyFeatures(
                teamMap, visitedTeams, F, active);
    }
    return numProgramsInPolicy;
}

/******************************************************************************/
void team::policyInstructions(
    map<long, team *> &teamMap, set<team *, teamIdComp> &visitedTeams,
    vector<int> &programInstructionCounts,
    vector<int> &effectiveProgramInstructionCounts) const {
    visitedTeams.insert(teamMap[id_]);

    for (auto prog : members_) {
        programInstructionCounts.push_back(prog->Size());
        effectiveProgramInstructionCounts.push_back(prog->SizeEffective());

        if (prog->action() >= 0 &&
            find(visitedTeams.begin(), visitedTeams.end(),
                 teamMap[prog->action()]) == visitedTeams.end())
            teamMap[prog->action()]->policyInstructions(
                teamMap, visitedTeams, programInstructionCounts,
                effectiveProgramInstructionCounts);
    }
}

///****************************************************************************/
// void team::getBehaviourSequence(vector<int>&s, int phase) {
//    vector < behaviourType > singleEpisodeBehaviour;
//    map < point *, double >::reverse_iterator rit;
//    for (rit=outcomes_.rbegin(); rit!=outcomes_.rend(); rit++){
//       if ((rit->first)->phase() == phase){
//          (rit->first)->getBehaviour(singleEpisodeBehaviour);
//          if (s.size() + singleEpisodeBehaviour.size() <=
//          MAX_NCD_PROFILE_SIZE)
//             s.insert(s.end(),singleEpisodeBehaviour.begin(),singleEpisodeBehaviour.end());
//             //will cast to int (only discrete values used)
//          else
//             break;
//       }
//    }
// }

/******************************************************************************/
double team::getMeanOutcome(int phase, int task, int auxDouble, bool allPhase,
                            bool allTask) {
    vector<double> outcomes;

    // for(auto ouiter = outcomes_[task][phase].begin(); ouiter !=
    // outcomes_[task][phase].end(); ouiter++){
    //    if (((allPhase || (ouiter->second)->phase() == phase) &&
    //             (allTask || (ouiter->second)->task() == task)))
    //       outcomes.push_back((ouiter->second)->auxDouble(auxDouble));

    //}

    for (auto ouiter1 = outcomes_.begin(); ouiter1 != outcomes_.end();
         ouiter1++) {  // task
        if (ouiter1->first != task && !allTask) continue;
        for (auto ouiter2 = ouiter1->second.begin();
             ouiter2 != ouiter1->second.end(); ouiter2++) {  // phase
            if (ouiter2->first != phase && !allPhase) continue;
            for (auto ouiter3 = ouiter2->second.begin();
                 ouiter3 != ouiter2->second.end(); ouiter3++)  // points
                outcomes.push_back(ouiter3->second->auxDouble(auxDouble));
        }
    }

    if (outcomes.size() == 0)
        die(__FILE__, __FUNCTION__, __LINE__,
            "trying to get meanOutcome with no outcomes");
    return accumulate(outcomes.begin(), outcomes.end(), 0.0) / outcomes.size();
}

/******************************************************************************/
double team::getMeanOutcome(int phase, int task, int auxDouble, int auxInt,
                            long auxIntMatch, bool allPhase, bool allTask) {
    vector<double> outcomes;

    // for(auto ouiter = outcomes_[task][phase].begin(); ouiter !=
    // outcomes_[task][phase].end(); ouiter++){
    //    if ((allPhase || (ouiter->second)->phase() == phase) && (allTask
    //    || (ouiter->second)->task() == task) &&
    //    (ouiter->second)->auxInt(auxInt) == auxIntMatch)
    //       outcomes.push_back((ouiter->second)->auxDouble(auxDouble));
    // }

    for (auto ouiter1 = outcomes_.begin(); ouiter1 != outcomes_.end();
         ouiter1++) {  // task
        if (ouiter1->first != task && !allTask) continue;
        for (auto ouiter2 = ouiter1->second.begin();
             ouiter2 != ouiter1->second.end(); ouiter2++) {  // phase
            if (ouiter2->first != phase && !allPhase) continue;
            for (auto ouiter3 = ouiter2->second.begin();
                 ouiter3 != ouiter2->second.end(); ouiter3++)  // points
                if (ouiter3->second->auxInt(auxInt) == auxIntMatch)
                    outcomes.push_back(ouiter3->second->auxDouble(auxDouble));
        }
    }

    if (outcomes.size() == 0)
        die(__FILE__, __FUNCTION__, __LINE__,
            "trying to get meanOutcome with no outcomes");
    return accumulate(outcomes.begin(), outcomes.end(), 0.0) /
           outcomes
               .size();  //+(int)(topPortion*outcomes.size()),0.0)/(int)(topPortion*outcomes.size());
}

///****************************************************************************/
// double team::getMeanOutcome(int phase, int task, int auxDouble, double
// &rValue1, double &rValue2, double minVal) {
//    vector < double > outcomes;
//    rValue2 = 0;
//    for(auto ouiter = outcomes_[task][phase].begin(); ouiter !=
//    outcomes_[task][phase].end(); ouiter++){
//       if (((phase == -1 || (ouiter->first)->phase() == phase) &&
//                (task == -1 || (ouiter->first)->task() == task) &&
//                (ouiter->first)->auxDouble(auxDouble) > minVal)){
//          outcomes.push_back((ouiter->first)->auxDouble(auxDouble));
//          if((ouiter->first)->auxDouble(auxDouble) >= 50)
//             rValue2++;
//       }
//
//    }
//    if (outcomes.size() == 0)
//       return 0.0;//die(__FILE__, __FUNCTION__, __LINE__, "trying to get
//       meanOutcome with no outcomes");
//    rValue1 = accumulate(outcomes.begin(),outcomes.end(), 0.0) /
//    outcomes.size();
//    //+(int)(topPortion*outcomes.size()),0.0)/(int)(topPortion*outcomes.size());
//    return rValue2 + rValue1/3000;
// }

/******************************************************************************/
bool team::hasPointDesc(string d, int task, int phase) {
    for (auto ouiter = outcomes_[task][phase].begin();
         ouiter != outcomes_[task][phase].end(); ouiter++)
        if (d.compare((ouiter->second)->desc()) == 0) return true;
    return false;
}

/******************************************************************************/
double team::getPointDescScore(string d, int task, int phase, int auxDouble) {
    for (auto ouiter = outcomes_[task][phase].begin();
         ouiter != outcomes_[task][phase].end(); ouiter++)
        if (d.compare((ouiter->second)->desc()) == 0)
            return (ouiter->second)->auxDouble(auxDouble);
    die(__FILE__, __FUNCTION__, __LINE__,
        "trying to get score from point that doesn't exist");
    return 0;
}

///****************************************************************************/
// bool team::getOutcome(point *pt, double *out) {
//    map < point *, double, pointLexicalLessThan > :: iterator ouiter;
//
//    if((ouiter = outcomes_.find(pt)) == outcomes_.end())
//       return false;
//
//    *out = ouiter->second;
//
//    return true;
// }

///****************************************************************************/
// double team::getRMSOutcome(int phase, int auxDouble) {
//    double rms = 0;
//    for(auto ouiter = outcomes_.begin(); ouiter != outcomes_.end();
//    ouiter++)
//       if ((ouiter->first)->phase() == phase)
//          rms += (ouiter->first)->auxDouble(auxDouble);
//    return isfinite(rms) ? -(sqrt(rms / outcomes_.size())) :
//    -numeric_limits<double>::max();
// }

///****************************************************************************/
// bool team::hasOutcome(point *pt) {
//    map < point *, double, pointLexicalLessThan > :: iterator ouiter;
//
//    if((ouiter = outcomes_.find(pt)) == outcomes_.end())
//       return false;
//
//    return true;
// }

///****************************************************************************/
///* Calculate normalized compression distance w.r.t another team. */
// double team::ncdBehaviouralDistance(team * t, int phase) {
//    ostringstream oss;
//    vector <int> theirBehaviourSequence;
//    t->getBehaviourSequence(theirBehaviourSequence, phase);
//    vector <int> myBehaviourSequence;
//    getBehaviourSequence(myBehaviourSequence, phase);
//    if (myBehaviourSequence.size() == 0 || theirBehaviourSequence.size()
//    == 0)
//       return -1;
//    return
//    normalizedCompressionDistance(myBehaviourSequence,theirBehaviourSequence);
// }

/******************************************************************************/
int team::numOutcomes(int phase, int task) {
    int numOut = 0;
    // for(auto ouiter = outcomes_.begin(); ouiter != outcomes_.end();
    // ouiter++)
    //    if (ouiter->first->phase() == phase && (task == -1 ||
    //    ouiter->first->task() == task))
    //       numOut++;
    if (task < 0)
        for (auto ouiter1 = outcomes_.begin(); ouiter1 != outcomes_.end();
             ouiter1++)
            numOut += ouiter1->second[phase].size();
    else
        numOut = outcomes_[task][phase].size();
    return numOut;
}

/******************************************************************************/

// Assumes the program is in the team
// Does not maintain team size > 0
// Does not maintain n_atomic_ > 0
void team::RemoveProgram(program *prog) {
    if (prog->action() < 0) n_atomic_--;
    prog->nrefs_--;
    auto it = find(members_.begin(), members_.end(), prog);
    members_.erase(it);
    members_run_.resize(members_.size());  // put in mark introns
}

// Return true if a program was removed, otherwise return false
bool team::RemoveRandomProgram(mt19937 &rng) {
    if (members_.size() < 2) return false;  // Maintain team size > 0
    uniform_int_distribution<int> dis_programs(0, members_.size() - 1);
    auto it = members_.begin();
    advance(it, dis_programs(rng));
    // Don't remove the only atomic
    if (!((*it)->action() < 0 && n_atomic_ < 2)) {
        if ((*it)->action_ < 0) n_atomic_--;
        (*it)->nrefs_--;
        members_.erase(it);
        return true;
    }
    return false;
}

/******************************************************************************/
void team::resetOutcomes(int phase) {
    // map < point *, double, pointLexicalLessThan > :: iterator ouiter;
    // for (ouiter = outcomes_.begin(); ouiter != outcomes_.end();)
    //{
    //    if ((ouiter->first)->phase() == phase || phase < 0){
    //       delete ouiter->first;
    //       outcomes_.erase(ouiter++);
    //    }
    //    else
    //       ouiter++;
    // }
    //

    for (auto ouiter1 = outcomes_.begin(); ouiter1 != outcomes_.end();
         ouiter1++)  // task
        for (auto ouiter2 = ouiter1->second.begin();
             ouiter2 != ouiter1->second.end(); ouiter2++)  // phase
            if (ouiter2->first == phase || phase == -1) {
                for (auto ouiter3 = ouiter2->second.begin();
                     ouiter3 != ouiter2->second.end();) {
                    delete ouiter3->second;
                    ouiter2->second.erase(ouiter3++);
                }
            }
    quickSums_.clear();
    quickMeans_.clear();
    runTimeComplexityIns_ = 0;
    runTimeComplexityTms_ = 0;
}

/******************************************************************************/
void team::swapOutcomePhase(int phaseFrom, int phaseTo, int auxInt,
                            long auxIntMatch) {
    resetOutcomes(phaseTo);
    resetOutcomes(_TEST_PHASE);
    for (auto ouiter1 = outcomes_.begin(); ouiter1 != outcomes_.end();
         ouiter1++) {  // task
        for (auto ouiter2 = outcomes_[ouiter1->first][phaseFrom].begin();
             ouiter2 != outcomes_[ouiter1->first][phaseFrom].end();
             ouiter2++)  // points
            if ((ouiter2->second)->auxInt(auxInt) == auxIntMatch) {
                ouiter2->second->phase(phaseTo);
                setOutcome(ouiter2->second);
            }
        outcomes_[ouiter1->first][phaseFrom].clear();
    }
}

/******************************************************************************/
void team::setOutcome(point *pt) {
    // if((outcomes_[pt->task()][pt->phase()].insert(map <
    // pt->auxInt(POINT_AUX_INT_ENVSEED), point
    // *>::value_type(pt->auxInt(POINT_AUX_INT_ENVSEED),pt))).second ==
    // false) die(__FILE__, __FUNCTION__, __LINE__, "could not set outcome,
    // duplicate point?");
    outcomes_[pt->task()][pt->phase()][pt->auxInt(POINT_AUX_INT_ENVSEED)] = pt;
    if (hasQuickSum(pt->task(), pt->key(), pt->phase()))
        quickSums_[pt->task()][pt->key()][pt->phase()] +=
            pt->auxDouble(pt->key());
    else
        quickSums_[pt->task()][pt->key()][pt->phase()] =
            pt->auxDouble(pt->key());

    quickMeans_[pt->task()][pt->key()][pt->phase()] =
        quickSums_[pt->task()][pt->key()][pt->phase()] /
        numOutcomes(pt->phase(), pt->task());
}

/******************************************************************************/
program *team::getAction(state *s, map<long, team *> &teamMap,
                         bool updateActive,
                         set<team *, teamIdComp> &visitedTeams,
                         long &decisionInstructions, int timeStep,
                         vector<team *> &teamPath, mt19937 &rng,
                         bool &verbose) {
    //_depthSum += visitedTeams.size(); _visitedCount++;
    visitedTeams.insert(teamMap[id_]);
    teamPath.push_back(teamMap[id_]);

    int l = 0;
    for (auto prog : members_) {
        prog->bidVal(prog->Run(s, timeStep, visitedTeams.size(), verbose));
        members_run_[l++] = prog;
        decisionInstructions += prog->SizeEffective();
    }

    sort(members_run_.begin(), members_run_.end(), ProgramBidLexicalCompare());
    long teamIdToFollow = 0;
    for (size_t i = 0; i < members_run_.size(); i++) {
        if (members_run_[i]->action() < 0) {
            return members_run_[i];
        } else if (find(visitedTeams.begin(), visitedTeams.end(),
                        teamMap[members_run_[i]->action()]) ==
                   visitedTeams.end()) {
            teamIdToFollow = members_run_[i]->action();
            break;
        }
    }
    return teamMap[teamIdToFollow]->getAction(
        s, teamMap, updateActive, visitedTeams, decisionInstructions, timeStep,
        teamPath, rng, verbose);
}

/******************************************************************************/
program *team::getAction(
    state *s, map<long, team *> &teamMap, bool updateActive,
    set<team *, teamIdComp> &visitedTeams, long &decisionInstructions,
    int timeStep, vector<program *> &allPrograms,
    vector<program *> &winningPrograms, vector<set<long> > &decisionFeatures,
    // vector<set<memoryEigen *, memoryEigenIdComp> > &decisionMemories,
    vector<team *> &teamPath, mt19937 &rng, bool &verbose) {
    //_depthSum += visitedTeams.size(); _visitedCount++;
    visitedTeams.insert(teamMap[id_]);
    teamPath.push_back(teamMap[id_]);

    set<long> features;
    set<long> featuresSingle;
    // set<memoryEigen *, memoryEigenIdComp> memories;
    set<memoryEigen *, memoryEigenIdComp> memoriesSingle;

    int l = 0;
    for (auto prog : members_) {
        prog->bidVal(prog->Run(s, timeStep, visitedTeams.size(), verbose));
        allPrograms.push_back(prog);
        members_run_[l++] = prog;
        decisionInstructions += prog->SizeEffective();

        prog->features(featuresSingle);
        features.insert(featuresSingle.begin(), featuresSingle.end());
        // TODO(skelly): remove shared memory code
        //  for (int mem_t = 0; mem_t < memoryEigen::NUM_MEMORY_TYPES;
        //  mem_t++) {
        //    memories.insert(prog->MemGet(mem_t));
        //  }
    }
    decisionFeatures.push_back(features);
    // decisionMemories.push_back(memories);

    sort(members_run_.begin(), members_run_.end(), ProgramBidLexicalCompare());
    long teamIdToFollow = 0;
    for (size_t i = 0; i < members_run_.size(); i++) {
        if (members_run_[i]->action() < 0) {  // atomic
            winningPrograms.push_back(members_run_[i]);
            members_run_[i]->featuresMem(featuresSingle);
            decisionFeatures.push_back(featuresSingle);
            return members_run_[i];
        } else if (find(visitedTeams.begin(), visitedTeams.end(),
                        teamMap[members_run_[i]->action()]) ==
                   visitedTeams.end()) {
            teamIdToFollow = members_run_[i]->action();
            winningPrograms.push_back(members_run_[i]);
            members_run_[i]->featuresMem(featuresSingle);
            decisionFeatures.push_back(featuresSingle);
            // members_run_Tally[members_run_[i]->id_]++;
            break;
        }
    }
    return teamMap[teamIdToFollow]->getAction(
        s, teamMap, updateActive, visitedTeams, decisionInstructions, timeStep,
        allPrograms, winningPrograms, decisionFeatures, teamPath, rng, verbose);
}