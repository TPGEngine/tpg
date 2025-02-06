#ifndef RegisterMachine_h
#define RegisterMachine_h

#include "instruction.h"

#define MAX_GRAPH_DEPTH 1000.0

class RegisterMachine {
  public:
   bool from_string_;        // TODO(skelly):remove
   bool use_evolved_const_;  // Fixed parameter: whether to use constants
   bool stateful_;           // Fixed parameter: whether memories maintain state

   int action_;                 // Mutable: discrete action
   int obs_index_;              // Mutable: index into observation space
   int observation_buff_size_;  // Mutable: observation buff size

   std::vector<instruction *> instructions_;
   std::vector<instruction *> instructions_effective_;
   vector<int> op_counts_;  // Count each op in instructions_effective_

   // Vector storing 1 MemoryEigen* of each type (SCALAR, VECTOR, MATRIX)
   vector<MemoryEigen *> private_memory_;

   // Vector storing id of each private memory (required for ToString)
   vector<long> private_memory_ids_;

   // Vector storing 1 MemoryEigen* of each type (SCALAR, VECTOR, MATRIX)
   vector<MemoryEigen *> observation_memory_buff_;

   long id_;             // Unique id
   double bid_val_;      // Temporarily store the most recent bid value
   set<long> features_;  // Features indexed by this RegisterMachine
   long gtime_;          // Generation created
   int nrefs_;           // Number of references by teams

   // Create arbitrary RegisterMachine
   RegisterMachine(long action,
                   std::unordered_map<std::string, std::any> &params,
                   std::unordered_map<std::string, int> &state, mt19937 &rng,
                   std::vector<bool> &legalOps);

   // Create RegisterMachine and copy instructions
   RegisterMachine(long action, std::vector<instruction*> &instructions,
                   std::unordered_map<std::string, std::any>& params,
                   std::unordered_map<std::string, int>& state, mt19937& rng,
                   std::vector<bool>& legal_ops);

   // Copy contructor
   RegisterMachine(RegisterMachine &rm);

   // Copy assignment operator
   RegisterMachine &operator=(RegisterMachine &rm);

    // Clone RegisterMachine with new id
   RegisterMachine(RegisterMachine &plr,
                   std::unordered_map<std::string, std::any> &params,
                   std::unordered_map<std::string, int> &state);

   // Create RegisterMachine from checkpoint string
   RegisterMachine(std::vector<std::string> outcomeFields,
                   std::vector<std::map<long, MemoryEigen *>> &memory_maps,
                   std::unordered_map<std::string, std::any> &params,
                   mt19937 &rng);

   ~RegisterMachine();

   inline void AddToInputMemoryBuff(MatrixDynamic &mat, int mem_t) {
      observation_memory_buff_[mem_t]->working_memory_.push_front(mat);
      observation_memory_buff_[mem_t]->working_memory_.pop_back();
   }

   inline void ClearWorkingMemory() {
      for (auto memory : private_memory_) memory->ClearWorking();
      for (auto memory : observation_memory_buff_) memory->ClearWorking();
   }

   inline void CopyPrivateConstToWorkingMemory() {
      for (auto m : private_memory_) {
         m->CopyConstToWorking();
      }
   }

   void CopyObservationToMemoryBuff(state *obs, size_t memory_type);

   // Determine which observation variables of used in effetive instructions,
   // store in features_
   void MarkFeatures(instruction *istr, int in);

   // Determine which instructions are effective,
   // store in instructions_effective_
   void MarkIntrons(std::unordered_map<std::string, std::any> &params_);

   // Execute this register machine
   void Run(state *, int &time_step, const size_t &graph_depth, bool &verbose);

   std::string ToString(bool);
   std::string ToStringMemory();

   void Mutate(std::unordered_map<std::string, std::any> &params,
               std::unordered_map<std::string, int> &state, mt19937 &rng,
               std::vector<bool> &legal_ops);

   void MutateMemorySize(std::unordered_map<std::string, std::any> &params,
                         std::unordered_map<std::string, int> &state,
                         mt19937 &rng);

   void ResizeMemory(std::unordered_map<std::string, std::any> &params,
                     std::unordered_map<std::string, int> &state,
                     int new_memory_size);

   void SetupMemory(std::unordered_map<std::string, int> &state, int n_memories,
                    int memory_size);

   // Copy constants from prog to this register machine
   void CopyEvolvedConstants(RegisterMachine &prog) {
      for (size_t m = 0; m < private_memory_.size(); m++) {
         for (size_t i = 0; i < private_memory_[m]->n_memories_; i++) {
            private_memory_[m]->const_memory_[i] =
                prog.private_memory_[m]->const_memory_[i];
         }
      }
   }
};

struct RegisterMachineBidLexicalCompare {
   bool operator()(RegisterMachine *rm1, RegisterMachine *rm2) const {
      if (rm1->bid_val_ != rm2->bid_val_) {
         return rm1->bid_val_ > rm2->bid_val_;
      } else {
         return rm1->id_ < rm2->id_;
      }
   }
};

struct RegisterMachineIdComp {
   bool operator()(RegisterMachine *rm1, RegisterMachine *rm2) const {
      return rm1->id_ < rm2->id_;
   }
};

#endif
