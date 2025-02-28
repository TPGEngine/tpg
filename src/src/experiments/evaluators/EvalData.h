#ifndef EvalData_h
#define EvalData_h

#define REWARD1_IDX 0
#define VISITED_TEAMS_IDX 1
#define INSTRUCTIONS_IDX 2
#define REWARD2_IDX 3

#include <set>
#include <list>
#include <map>
#include "TaskEnv.h"
// #include "RegisterMachine.h"
#include "state.h"

class team;
class RegisterMachine;

/*******************************************************************************
 EvalData encapsulates all data relating to evaluation results.
 This simplifies data passing between evaluator processes.
*/
class EvalData {
    public:
    int tpg_seed;
    int world_rank;
    int world_size;
    int timestep;
    bool verbose;
    std::map<long, team *> team_map;
    std::vector<team *> teams;       // All teams to evaluate
    std::string checkpointString;    // For passing tpg data
    bool partially_observable;  // whether or not the task is partially
                                // observable

    /* Per individual eval data ***********************************************/
    ofstream dbg_out;
    team *tm;  // Current team to evaluate
    TaskEnv *task;
    std::string eval_result;       // For passing eval results between mpi procs
    int episode;              // Episode number
    std::vector<int> fingerprint;  // Behaviour fingerprint (usually not used)

    // double Stats to track for each evaluation
    // Currently this vec will contain:
    // [fitness, mean teams per prediction, mean instructions per prediction]
    std::vector<double> stats_double;

    // int Stats to track for each eavluation
    // Currently this vec will contain:
    // task number, phase, environment seed, internal test node id]
    std::vector<int> stats_int;

    // Programs which have won an auction in this eval
    // std::set<RegisterMachine *, RegisterMachineIdComp> active;

    /* Per episode data *******************************************************/
    int n_prediction;  // Number or predictions in eval (if known)
    // Prediction and target sequences (for recursive forecasting)
    std::vector<double> sequence_targ;
    std::vector<double> sequence_pred;

    /* Per step data **********************************************************/
    int sample;          // Sample or step number
    int save_frame = 0;  // Whether to save animation frames
    std::set<team*> teams_visited;
    std::vector<team *>
        team_path;  // Execution path (overwritten for each prediction)
    // Number of instructions executed per prediction (overwritten for each
    // prediction)
    long instruction_count;
    // Program which defines the output in the current timestep
    RegisterMachine *program_out;
    bool animate;            // whether or not to draw animation
    state *obs;              // Observation input
    std::list<double> obs_list;   // Obs as a list (tmp use only)
    std::vector<double> obs_vec;  // Obs as a vec (tmp use only)

    EvalData(){};

    void AccumulateStepData();
};

#endif