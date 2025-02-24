// TODO(skelly): clean this clean up file!
#ifndef TaskEnv_h
#define TaskEnv_h

#include <cstring>
#include <deque>
#include <random>

class TaskEnv {
   public:
    std::string eval_type_;
    std::vector<double> state_;     // state variables 
    std::vector<double> state_po_;  // state variables (partially observable)
    // discrete actions map to an index into actionDiscrete
    std::vector<double> actionsDiscrete;
    int previousActionDiscrete;
    double previousActionContinuous;
    double reward;
    int step_;
    int max_step_;
    bool terminalState;
    // A min reward is useful for filtering very bad or infinitely bad rewards
    // in time series tasks
    double min_reward_;
    int n_eval_train_;
    int n_eval_validation_;
    int n_eval_test_;

    struct Results {
        double r1;
        double r2;
    };

    TaskEnv() { 
        terminalState = false;
    }
    virtual ~TaskEnv() {}

    std::vector<double> &GetObsVec(bool po) { return po ? state_po_ : state_; }
    double GetObsVar(int var, bool po) {
        return po ? state_po_[var] : state_[var];
    }
    void SetStateVar(int var, double v) {
        state_po_[var] = v;
        state_[var] = v;
    }
    inline std::string EvalType() const { return eval_type_; }
    virtual bool DiscreteActions() const { return true; }
    virtual double MinActionContinuous() const { return 0.0; }
    virtual double MaxActionContinuous() const { return 0.0; }
    virtual void Reset(std::mt19937 &) { step_ = 0; }
    virtual Results Update(int, double, std::mt19937 &) { return {0.0, 0.0}; };
    Results SimStep(std::vector<double> &action) { return {0.0, 0.0}; }
    virtual bool Terminal() { return false; }
    int GetNumEval(int phase) {
        if (phase == 0)
            return n_eval_train_;
        else if (phase == 1)
            return n_eval_validation_;
        else
            return n_eval_test_;
    }
};
#endif
