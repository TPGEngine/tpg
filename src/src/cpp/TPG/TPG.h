#ifndef TPG_H
#define TPG_H
#include <TaskEnv.h>

#include <any>
#include <iomanip>
#include <random>

#include "RegisterMachine.h"
#include "api_client.h"
#include "instruction.h"
#include "MemoryEigen.h"
#include "point.h"
#include "state.h"
#include "team.h"

#define NUM_RNG 2
#define TPG_SEED 0
#define AUX_SEED 1

class TPG {
   public:
    TPG();
    TPG(const TPG &);
    ~TPG();

    void AddProgram(RegisterMachine *p);
    void AddTeam(team *tm);
    void RemoveTeam(team *tm);
    void AddMemory(long prog_id, MemoryEigen *m);
    team *getTeamByID(long id);
    bool haveEliteTeam(string taskset, int fitMode, int phase);
    void Seed(size_t i, uint_fast32_t s);
    void InitExperimentTracking(APIClient *apiClient);

    /***************************************************************************
     * Methods to implement the TPG algorithm.
     **************************************************************************/
    void checkRefCounts(const char *);
    void CleanupProgramsWithNoRefs();
    void clearMemory();
    void countRefs();
    void finalize();
    void genSampleSets(size_t);
    team* TeamSelector_Tournament(vector<team*> &candidate_parent_teams);
    void GenerateNewTeams();
    void TeamMutator_ProgramOrder(team *team_to_mu);
    void TeamMutator_AddPrograms(team *team_to_mu);
    void TeamMutator_RemovePrograms(team *team_to_mu);
    team *CloneTeam(team *team_to_clone);
    RegisterMachine *CloneProgram(RegisterMachine *prog);
    // void ProgramMutator_Memory(RegisterMachine *&prog_to_mu);
    void ProgramMutator_Instructions(RegisterMachine *prog_to_mu);
    void ProgramMutator_ActionPointer(RegisterMachine *prog_to_mu, team *new_team,
                                      int &n_new_teams);
    void AddAncestorToPhylogeny(team *parent, team *new_team);
    void AddTeamToPhylogeny(team *new_team);
    void ApplyVariationOps(team *team_to_modify, int &n_new_teams);
    team *genTeamsInternal(long, mt19937 &, set<team *, teamIdComp> &,
                           map<long, team *> &);
    int genUniqueProgram(RegisterMachine *, set<RegisterMachine *, RegisterMachineIdComp>);
    RegisterMachine *getAction(team *tm, state *s, bool updateActive,
                       set<team *, teamIdComp> &visitedTeams,
                       long &decisionInstructions, int timeStep,
                       vector<team *> &teamPath, mt19937 &rng, bool verbose);

    RegisterMachine *getAction(
        team *tm, state *s, bool updateActive,
        set<team *, teamIdComp> &visitedTeams, long &decisionInstructions,
        int timeStep, vector<RegisterMachine *> &allPrograms,
        vector<RegisterMachine *> &winningPrograms, vector<set<long>> &decisionFeatures,
        // vector<set<MemoryEigen *, MemoryEigenIdComp>> &decisionMemories,
        vector<team *> &teamPath, mt19937 &rng, bool verbose);
    void GetAllNodes(team *tm, set<team *, teamIdComp> &teams,
                     set<RegisterMachine *, RegisterMachineIdComp> &RegisterMachines);
    // void GetAllNodes(team *tm, set<team *, teamIdComp> &teams,
    //                  set<RegisterMachine *, RegisterMachineIdComp> &RegisterMachines,
    //                  set<MemoryEigen *, MemoryEigenIdComp> &memories);


    team *GetBestTeam();
    set<team*, teamFitnessLexicalCompare> GetRootTeamsInSet();
    vector<team *> GetRootTeamsInVec() const;
    map<long, team *> GetRootTeamsInMap() const;
    void InitTeams();
    void internalReplacementPareto(int, int, int, team *, map<long, team *> &,
                                   set<team *, teamIdComp> &, mt19937 &);
    bool isElitePS(team *tm, int phase);
    void MarkEffectiveCode();
    void GetPolicyFeatures(int, set<long> &, bool);
    // void printGraphDot(
    //     team *, size_t frame, int episode, int step, size_t depth,
    //     vector<RegisterMachine *> allPrograms, vector<RegisterMachine *> winningPrograms,
    //     vector<set<long>> decisionFeatures,
    //     vector<set<MemoryEigen *, MemoryEigenIdComp>> decisionMemories,
    //     vector<team *> teamPath, bool drawPath,
    //     set<team *, teamIdComp> visitedTeamsAllTasks);
    // void printGraphDotGPEM(long rootTeamId, map<long, string> &teamColMap,
    //                        set<team *, teamIdComp> &visitedTeamsAllTasks,
    //                        vector<map<long, double>> &teamUseMapPerTask);
    void printGraphDotGPTPXXI(long rootTeamID,
                              set<team *, teamIdComp> &visitedTeamsAllTasks,
                              vector<map<long, double>> &teamUseMapPerTask,
                              vector<int> &steps_per_task);
    // void printGraphDotGPEMAnimate(long rootTeamId, size_t frame, int episode,
    //                               int step, size_t depth,
    //                               vector<RegisterMachine *> allPrograms,
    //                               vector<RegisterMachine *> winningPrograms,
    //                               set<team *, teamIdComp>
    //                               &visitedTeamsAllTasks, vector<map<long,
    //                               double>> &teamUseMapPerTask, vector<team *>
    //                               teamPath);
    void printHostGraphsDFS(long, long);
    void printHostGraphsHostsOnly(long, long);
    void printPhyloGraphDot(team *);
    void printProgramInfo();
    void printOss();
    void printOss(ostringstream &o);
    void printTeamInfo(long, int, bool, bool, long teamId = -1);
    void trackTeamInfo(long, int, bool, long teamId = -1);
    
    void RegisterMachineCrossover(RegisterMachine *p1, RegisterMachine *p2,
                          RegisterMachine **c1, RegisterMachine **c2);


    void ReadCheckpoint(long, int, int, bool, const string &);

    void ReadParameters(string file_name,
                        std::unordered_map<string, std::any> &params);
    void recalculateProgramRefs();
    void SanityCheck();
    void TeamSizesMatchProgRefs();  // Sanity check.
    inline void resetOutcomes(int phase, bool roots);
    void SelectTeams();
    team *TeamCrossover(team* parent1, team* parent2);
    void UpdateTeamPhyloData(team *tm);
    void FindSingleTaskFitnessRange(vector<TaskEnv *> &tasks,
                                    vector<vector<double>> &mins,
                                    vector<vector<double>> &maxs);
    vector<team *> NormalizeScoresAndRankTeams(
        vector<TaskEnv *> &tasks, vector<int> &set,
        vector<vector<double>> &min_scores, vector<vector<double>> &max_scores);
    void FindMultiTaskElites(vector<TaskEnv *> &tasks,
                             vector<vector<double>> &min_scores,
                             vector<vector<double>> &max_scores);
    void SetEliteTeams(vector<TaskEnv *> &tasks);
    void setOutcome(team *tm, string behav, vector<double> &rewards,
                    vector<int> &ints, long gtime);
    std::string PhylogenyToString();
    inline void teamMap(map<long, team *> &team_map) const {
        team_map = _teamMap;
    }
    // void teamTaskRank(int, const vector<int> &);
    void updateMODESFilters(bool);
    void WriteCheckpoint(long, bool);
    void WriteMPICheckpoint(string &, vector<team *> &);

    /*****************************************************************************
     *  TPG member variables and data structures.
     ****************************************************************************/
    //  Populations
    set<team *, teamIdComp> team_pop_;      // Teams
    // Map team id -> team* for RegisterMachine graph traversal
    map<long, team *> _teamMap;
    // keep track of which teams are elites wrt each taskSet
    map<string, vector<team *>> task_set_map_;
    map<long, RegisterMachine *> program_pop_;
    vector<vector<long>> _Memids;
    // one map for each memory type: id->memory*
    vector<map<long, MemoryEigen *>> _Memory;

    // Phylogeny data
    map<long, phyloRecord> phylo_graph_;
    map<long, modesRecord> _allComponentsA;
    map<long, modesRecord> _allComponentsAt;
    map<long, modesRecord> _persistenceFilterA;
    map<long, modesRecord> _persistenceFilterA_t;
    map<long, modesRecord> _persistenceFilterAllTime;

    vector<bool> _ops;  // legel RegisterMachine operations
    set<team *, teamIdComp> _eliteTeams;
    // keys: taskSet, fitMode, phase
    map<string, map<int, map<int, team *>>> _eliteTeamPS;
    set<long> elite_team_id_history_;
    vector<mt19937> rngs_;
    vector<uint_fast32_t> seeds_;
    vector<int> n_input_;                // Number of inputs per task.
    vector<int> observation_buff_size_;  // Observaiton buff size per task.
    ostringstream oss;                   // logging, reporting
    vector<size_t> _numEliteTeamsCurrent;

    uniform_real_distribution<> real_dist_;  // random reals in [0,1]

    // API client for tracking experiments
    APIClient *api_client_;

   public:
    std::unordered_map<std::string, std::any> params_;
    template <typename T>
    T GetParam(string p) {
        return std::any_cast<T>(params_[p]);
    }
    bool HaveParam(string p) { return params_.find(p) != params_.end(); }
    std::unordered_map<std::string, int> state_;
    int GetState(string p) { return state_[p]; }
    void ProcessParams();
    void SetParams(int argc, char **argv);
    void MutateActionToTerminal(RegisterMachine *prog_to_mu, team *new_team);
    void MutateActionToTeam(RegisterMachine *prog_to_mu, team *new_team,
                            int &n_new_teams);
};

#endif
