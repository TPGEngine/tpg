#ifndef team_h
#define team_h

#include <list>
#include <map>
#include <queue>
#include <set>
#include <unordered_set>

#include "misc.h"
#include "point.h"
#include "state.h"
#include "RegisterMachine.h"


#define MEMBERS_RUN_ENTROPY_INDEX 3
#define NUM_TEAM_DISTANCE_MEASURES 3
// arbitrarily large, greater than max ALE frames/episode of 18000
#define MAX_NCD_PROFILE_SIZE 20000

using namespace std;

class EvalData;
struct teamIdComp;

class team {
 public:
  inline void addAncestorId(long aid) { ancestorIds_.push_back(aid); }
  inline void addEvalSeed(int s) { evalSeeds_.push_back(s); }
  inline void clearEvalSeeds() { evalSeeds_.clear(); }
  void InitMemory(map<long, team *> &, std::unordered_map<std::string, std::any> &params);
  inline void getAncestorIds(vector<long> &a) { a = ancestorIds_; }
  inline void setAncestorIds(vector<long> &a) { ancestorIds_ = a; }
  inline int numAncestorIds() { return ancestorIds_.size(); }
  inline bool elite(int phase) const { return elite_[phase]; }
  inline void elite(int phase, bool e) { elite_[phase] = e; }
  inline string getBehaviourString(int seed, int phase) {
    string s = "";
    for (size_t task = 0; task < outcomes_.size(); task++)
      if (outcomes_[task][phase].find(seed) != outcomes_[task][phase].end())
        s += outcomes_[task][phase][seed]->getBehaviourString();
    return s;
  }
  bool hasPointDesc(string, int, int);
  double getPointDescScore(string, int, int, int);
  void features(set<long> &) const;
  inline string fitnessBin() const { return (fitnessBins_.rbegin())->second; }
  inline string fitnessBin(long t) const {
    auto search = fitnessBins_.find(t);
    if (search != fitnessBins_.end())
      return search->second;
    else
      return "NA";  // code for not found
  }
  inline void fitnessBin(long t, string task) { fitnessBins_[t] = task; }
  inline void fitnessBins(map<long, string> &fb) {
    fitnessBins_.clear();
    fitnessBins_.insert(fb.begin(), fb.end());
  }
  inline map<long, string> fitnessBins() { return fitnessBins_; }
  void GetAllMemories(map<long, team *> &, set<team *, teamIdComp> &,
                      set<MemoryEigen *> &) const;
  void GetAllNodes(map<long, team *> &teamMap, set<team *, teamIdComp> &, long,
                   bool) const;
  void GetAllNodes(map<long, team *> &teamMap, set<team *, teamIdComp> &,
                   set<RegisterMachine *, RegisterMachineIdComp> &) const;
  void GetAllNodes(map<long, team *> &teamMap, set<team *, teamIdComp> &,
                   set<RegisterMachine *, RegisterMachineIdComp> &, set<MemoryEigen*> &) const;
  void getBehaviourSequence(vector<int> &, int);
  double GetMeanOutcome(int, int, int);
  double GetMeanOutcome(int, int, int, int, long);
  double GetMedianOutcome(int, int, int);
  inline bool hasOutcome(int task, int phase, int seed) {
    return outcomes_[task][phase].find(seed) != outcomes_[task][phase].end();
  }
  inline int inDeg() const { return incomingPrograms_.size(); }
  inline void AddIncomingProgram(long id) {
    incomingPrograms_.insert(id);
    root_ = false;
  }
  inline void removeIncomingProgram(long id) { incomingPrograms_.erase(id); }
  inline void incomingPrograms(set<long> &icp) const {
    icp = incomingPrograms_;
  }
  set<RegisterMachine *, RegisterMachineIdComp> CopyMembers() {
    set<RegisterMachine *, RegisterMachineIdComp> m;
    copy(members_.begin(), members_.end(), inserter(m, m.end()));
    return m;
  }
  // inline void SetMembers(list<RegisterMachine *> &m) {
  //   members_.clear();
  //   members_.assign(m.begin(), m.end());
  //  }
  inline int numEffectiveInstructions() const {
    return numEffectiveInstructions_;
  }
  double novelty(int, int) const;
  // Number of outcomes
  int numOutcomes(int phase, int task);
  // Get all outcomes from a particular phase
  // void outcomes(map<point*, double, pointLexicalLessThan>&, int);
  // Get all outcomes from a particular phase
  inline void outcomes(map<int, map<int, map<int, point *>>> &o) {
    o = outcomes_;
  }
  // void outcomes(int, int, vector < double >&); /* Get all outcome values of a
  // particular type and from a particular phase.*/
  inline void policyFeaturesGet(set<long> &f, bool active) const {
    if (active)
      f = policyFeaturesActive_;
    else
      f = policyFeatures_;
  }
  inline void policyFeaturesSet(set<long> &f, bool active) {
    if (active)
      policyFeaturesActive_ = f;
    else
      policyFeatures_ = f;
  }
  inline set<long> policyRootIds() const { return policyRootIds_; }
  inline void policyRootIds(set<long> &prids) const { prids = policyRootIds_; }
  inline void resetPolicyRootIds() { policyRootIds_.clear(); }
  inline void addPolicyRootIds(set<long> ptids) {
    policyRootIds_.insert(ptids.begin(), ptids.end());
  }
  inline void addPolicyRootId(long rootId) { policyRootIds_.insert(rootId); }
  void updatePolicyRoot(map<long, team *> &teamMap, set<team *, teamIdComp> &,
                        long &);
  int policyFeatures(
      map<long, team *> &teamMap, set<team *, teamIdComp> &, set<long> &,
      bool) const;  // populates last set with features and returns number of
                    // nodes(RegisterMachines) in policy
  void policyInstructions(map<long, team *> &, set<team *, teamIdComp> &,
                          vector<int> &, vector<int> &) const;
  void RemoveProgram(RegisterMachine *prog);
  bool RemoveRandomProgram(mt19937 &rng);
  void resetOutcomes(int); /* Delete all outcomes from phase. */
  inline bool root() const { return incomingPrograms_.size() == 0; }
  inline double runTimeComplexityIns() const { return runTimeComplexityIns_; }
  inline void runTimeComplexityIns(double rtc) { runTimeComplexityIns_ = rtc; }
  inline double runTimeComplexityTms() const { return runTimeComplexityTms_; }
  inline void runTimeComplexityTms(double rtc) { runTimeComplexityTms_ = rtc; }
  inline bool hasQuickMean(int task, int fitMode, int phase) {
    return quickMeans_.count(task) > 0 &&
           quickMeans_[task].count(fitMode) > 0 &&
           quickMeans_[task][fitMode].count(phase) > 0;
  }
  inline bool hasQuickSum(int task, int fitMode, int phase) {
    return quickSums_.count(task) > 0 && quickSums_[task].count(fitMode) > 0 &&
           quickSums_[task][fitMode].count(phase) > 0;
  }
  inline void setQuickMean(int task, int fitMode, int phase, double m) {
    quickMeans_[task][fitMode][phase] = m;
  }
  inline double getQuickMean(int task, int fitMode, int phase) {
    return quickMeans_[task][fitMode][phase];
  }
  void setOutcome(point *);
  inline int size() const { return members_.size(); }
  double symbiontUtilityDistance(team *) const;
  double symbiontUtilityDistance(vector<long> &) const;
  void swapOutcomePhase(int, int, int, long);
  inline void MuProgramOrder(int source, int destination) {
    auto leiter_source = members_.begin();
    auto leiter_destination = members_.begin();
    advance(leiter_source, source);
    advance(leiter_destination, destination);
    members_.splice(leiter_destination, members_, leiter_source);
  }

  void updateComplexityRecord(map<long, team *> &, int);
  void updateComplexityRecord(map<long, team *> &, int, int, long, int);

  string taskCode() const { return task_code_; }
  void taskCode(string tc) { task_code_ = tc; }

  // this constructor used for initialization and checkpointing
  team(long gtime, long id) {
    cloneId_ = -1;
    clones_ = 0;
    //_depthSum = 0;
    for (int i = 0; i < _NUM_PHASE; i++) elite_.push_back(false);
    //_visitedCount = 0;
    domBy_ = -1;
    domOf_ = -1;
    fit_ = 0.0;
    gtime_ = gtime;
    id_ = id;
    key_ = 0;
    lastCompareFactor_ = -1;
    n_atomic_ = 0;
    _n_eval = 0;
    numLinearM_ = 0;
    root_ = true;
    runTimeComplexityIns_ = 0;
    runTimeComplexityTms_ = 0;
  };

  ~team() {  // TODO(skelly) clean outcome data structure
    for (auto ouiter1 = outcomes_.begin(); ouiter1 != outcomes_.end();
         ouiter1++) {
      for (auto ouiter2 = ouiter1->second.begin();
           ouiter2 != ouiter1->second.end(); ouiter2++) {
        for (auto ouiter3 = ouiter2->second.begin();
             ouiter3 != ouiter2->second.end();) {
          delete ouiter3->second;
          ouiter2->second.erase(ouiter3++);
        }
      }
    }
  }

  inline void addDistance(int type, double d) {
    if (type == 0)
      distances_0_.insert(d);
    else if (type == 1)
      distances_1_.insert(d);
    else if (type == 2)
      distances_2_.insert(d);
  }
  void AddProgram(RegisterMachine *, int position = -1);
  // bool AddProgramActive(RegisterMachine *);
  string ToString() const;
  void clone(map<long, phyloRecord> &, team **);
  inline void clearDistances() {
    distances_0_.clear();
    distances_1_.clear();
    distances_2_.clear();
  }
  // void deleteOutcome(point *); /* Delete outcome. */
  void GetAction(EvalData& eval_data);
  // double ncdBehaviouralDistance(team*, int);
  void Shuffle(mt19937 &rng) {
    vector<RegisterMachine *> vec(members_.begin(), members_.end());
    shuffle(vec.begin(), vec.end(), rng);
    list<RegisterMachine *> shuffled_list{vec.begin(), vec.end()};
    members_.swap(shuffled_list);
  }
  void updateActiveMembersFromIds(vector<long> &);

  // protected:
  vector<long> ancestorIds_;
  long cloneId_;
  long clones_;
  vector<bool> elite_;
  multiset<double> distances_0_;
  multiset<double> distances_1_;
  multiset<double> distances_2_;
  int domBy_; /* Number of other teams dominating this team. */
  int domOf_; /* Number of other teams that this team dominates. */
  vector<int> evalSeeds_;
  // map of t -> task# keeps track of which fitness bin this
  // team was protected by in each gen (-1 for multi-task)
  map<long, string> fitnessBins_;
  double fit_; /* Fitness value used for selection */
  double fitProb_;
  long gtime_; /* Time step at which generated. */
  long id_;    /* Unique id of team. */
  set<long> incomingPrograms_;
  double key_; /* For sorting. */
  int lastCompareFactor_;
  std::list<RegisterMachine *> members_;   // team members for fast variation
  vector<RegisterMachine *> members_run_;  // team members for fast direct access
  int n_atomic_;
  int _n_eval;
  int numLinearM_;
  int numActiveTeams_;
  int numActivePrograms_;
  int numEffectiveInstructions_;
  int numActiveFeatures_;
  // TODO(skelly): simplify this data structure
  // Maps point[task][phase][envSeed] -> outcome
  map<int, map<int, map<int, point *>>> outcomes_;

  set<long> policyFeatures_;
  set<long> policyFeaturesActive_;
  set<long> policyRootIds_;
  map<int, map<int, map<int, double>>> quickMeans_;
  map<int, map<int, map<int, double>>> quickSums_;
  bool root_;
  double runTimeComplexityIns_;
  double runTimeComplexityTms_;
  string task_code_;
};

struct teamIdComp {
  bool operator()(team *t1, team *t2) const { return t1->id_ > t2->id_; }
};

struct teamFitnessCompare {
  bool operator()(team *t1, team *t2) const {
    // double d1, d2;
    if (!isEqual(t1->fit_, t2->fit_)) {
      return t1->fit_ > t2->fit_;
    } else {
      return t1->id_ > t2->id_;
    }
  }
};

struct teamFitnessLexicalCompare {
  bool operator()(team *t1, team *t2) const {
    // double d1, d2;
    if (!isEqual(t1->fit_, t2->fit_)) {
      t1->lastCompareFactor_ = 1;
      t2->lastCompareFactor_ = 1;
      return t1->fit_ > t2->fit_;
    } else {
      t1->lastCompareFactor_ = 7;
      t2->lastCompareFactor_ = 7;

      // TODO(skelly): potential dramatic effect on neutrality & evolution!
      // return t1->id_ > t2->id_;    // Younger is better
      return t1->id_ < t2->id_;  // Older is better
    }
  }
};

struct teamFitComplexLexCompare {
  bool operator()(team *t1, team *t2) const {
    if (!isEqual(t1->fit_, t2->fit_)) {
      t1->lastCompareFactor_ = 1;
      t2->lastCompareFactor_ = 1;
      return t1->fit_ > t2->fit_;
    } else {
      auto t1_val = t1->runTimeComplexityIns_;
      auto t2_val = t2->runTimeComplexityIns_;
      // auto t1_val = t1->runTimeComplexityTms_;
      // auto t2_val = t2->runTimeComplexityTms_;
      if (!isnan(t1_val) && !isnan(t2_val) && !isEqual(t1_val, t2_val)) {
        // cerr <<"teamFitComplexLexCompare 0:" << t1->fit_ << " " << t2->fit_
        // <<
        // endl;
        return t1_val < t2_val;
      } else {
        // cerr <<"teamFitComplexLexCompare 2:" << t1->id_ << " " << t2->id_ <<
        // endl;
        return t1->id_ > t2->id_;
      }
    }
  }
};

struct teamInDegCompare {
  bool operator()(team *t1, team *t2) { return t1->inDeg() < t2->inDeg(); }
};

#endif
