#include "RegisterMachine.h"

#include <stacktrace>

/******************************************************************************/
std::string RegisterMachine::ToString(bool effective_only) {
   std::ostringstream oss;
   oss << "RegisterMachine:" << id_ << ":" << gtime_ << ":" << action_ << ":"
       << stateful_ << ":" << nrefs_ << ":" << observation_buff_size_ << ":"
       << obs_index_;
   // for (auto &i : private_memory_ids_) oss << ":" << i;
   auto prog = effective_only ? instructions_effective_ : instructions_;
   for (auto istr : prog) oss << ":" << istr->checkpoint();
   oss << endl;
   for (auto *m : private_memory_) oss << m->ToString(id_);
   return oss.str();
}

/******************************************************************************/
std::string RegisterMachine::ToStringMemory() {
   std::ostringstream oss;
   for (auto *m : private_memory_) oss << m->ToString(id_);
   return oss.str();
}

/******************************************************************************/
// Create arbitrary RegisterMachine
RegisterMachine::RegisterMachine(
    long action, std::unordered_map<std::string, std::any> &params,
    std::unordered_map<std::string, int> &state, mt19937 &rng,
    std::vector<bool> &legal_ops) {
   action_ = action;
   stateful_ = std::any_cast<int>(params["stateful"]);
   gtime_ = state["t_current"];
   id_ = state["program_count"]++;
   nrefs_ = 0;
   observation_buff_size_ = std::any_cast<int>(params["observation_buff_size"]);
   obs_index_ = 0;
   uniform_int_distribution<int> disP(
       1, std::any_cast<int>(params["max_initial_prog_size"]));
   int prog_size = disP(rng);
   for (int i = 0; i < prog_size; i++) {
      auto in = new instruction(params, rng);
      in->Mutate(true, legal_ops, observation_buff_size_, rng);
      instructions_.push_back(in);
   }
   op_counts_.resize(instruction::NUM_OP);
   if (!isEqual(std::any_cast<double>(params["p_memory_mu_const"]),
                0.0)) {
      use_evolved_const_ = true;
   }
   SetupMemory(state, std::any_cast<int>(params["n_memories"]),
               std::any_cast<int>(params["memory_size"]));
}

// Create RegisterMachine and copy instructions
RegisterMachine::RegisterMachine(
    long action, std::vector<instruction*> &instructions,
    std::unordered_map<std::string, std::any> &params,
    std::unordered_map<std::string, int> &state, mt19937 &rng,
    std::vector<bool> &legal_ops) {
   action_ = action;
   stateful_ = std::any_cast<int>(params["stateful"]);
   gtime_ = state["t_current"];
   id_ = state["program_count"]++;
   nrefs_ = 0;
   observation_buff_size_ = std::any_cast<int>(params["observation_buff_size"]);
   obs_index_ = 0;
   for (auto i : instructions) {
      instructions_.push_back(new instruction(*i));
   }
   op_counts_.resize(instruction::NUM_OP);
   if (!isEqual(std::any_cast<double>(params["p_memory_mu_const"]),
                0.0)) {
      use_evolved_const_ = true;
   }
   SetupMemory(state, std::any_cast<int>(params["n_memories"]),
               std::any_cast<int>(params["memory_size"]));
}

// Copy Contructor
RegisterMachine::RegisterMachine(RegisterMachine &rm) {
   action_ = rm.action_;
   gtime_ = rm.gtime_;
   id_ = rm.id_;
   obs_index_ = rm.obs_index_;
   bid_val_ = -(numeric_limits<double>::max());
   nrefs_ = 0;
   stateful_ = rm.stateful_;
   observation_buff_size_ = rm.observation_buff_size_;
   use_evolved_const_ = rm.use_evolved_const_;
   op_counts_.resize(instruction::NUM_OP);
   for (size_t i = 0; i < MemoryEigen::kNumMemoryType_; i++) {
      private_memory_.push_back(new MemoryEigen(*(rm.private_memory_[i])));
      observation_memory_buff_.push_back(
          new MemoryEigen(*(rm.observation_memory_buff_[i])));
   }
   private_memory_ids_ = rm.private_memory_ids_;
   for (auto i : rm.instructions_) {
      instructions_.push_back(new instruction(*i));
   }
}

// Clone RegisterMachine with new id
RegisterMachine::RegisterMachine(
    RegisterMachine &rm, std::unordered_map<std::string, std::any> &params,
    std::unordered_map<std::string, int> &state) {
   action_ = rm.action_;
   gtime_ = state["t_current"];
   id_ = state["program_count"]++;
   obs_index_ = rm.obs_index_;
   bid_val_ = -(numeric_limits<double>::max());
   nrefs_ = 0;
   stateful_ = rm.stateful_;
   observation_buff_size_ = rm.observation_buff_size_;
   use_evolved_const_ = rm.use_evolved_const_;
   op_counts_.resize(instruction::NUM_OP);
   for (auto instr : rm.instructions_) {
      instructions_.push_back(new instruction(*instr));
   }
   SetupMemory(state, std::any_cast<int>(params["n_memories"]),
               rm.private_memory_[MemoryEigen::kScalarType_]->memory_size_);
      CopyEvolvedConstants(rm);
}

// Create RegisterMachine from string
RegisterMachine::RegisterMachine(
    std::vector<std::string> outcomeFields,
    std::vector<std::map<long, MemoryEigen *>> &memory_maps,
    std::unordered_map<std::string, std::any> &params, mt19937 &rng) {
   int f = 1;
   id_ = atoi(outcomeFields[f++].c_str());
   gtime_ = atoi(outcomeFields[f++].c_str());
   action_ = atoi(outcomeFields[f++].c_str());
   stateful_ = atoi(outcomeFields[f++].c_str());
   nrefs_ = atoi(outcomeFields[f++].c_str());
   observation_buff_size_ = atoi(outcomeFields[f++].c_str());
   obs_index_ = atoi(outcomeFields[f++].c_str());   
   // SetupMemry() but from existing memory pointers
   for (size_t mem_t = 0; mem_t < MemoryEigen::kNumMemoryType_; mem_t++) {
      // long id = atoi(outcomeFields[f++].c_str());
      private_memory_.push_back(memory_maps[mem_t][id_]);
      // private_memory_ids_.push_back(id);
      auto memory_size = memory_maps[mem_t][id_]->memory_size_;
      observation_memory_buff_.push_back(
          new MemoryEigen(mem_t, observation_buff_size_, memory_size));
   }
   for (size_t ii = f; ii < outcomeFields.size(); ii++) {
      vector<string> instructionString;
      SplitString(outcomeFields[ii], '_', instructionString);
      instruction *in = new instruction(params, rng);
      in->in1Src_ = stringToInt(instructionString[0]);
      in->in2Src_ = stringToInt(instructionString[1]);
      in->outIdx_ = stringToInt(instructionString[2]);
      in->op_ = stringToInt(instructionString[3]);
      in->in0Idx_ = stringToInt(instructionString[4]);
      in->in1Idx_ = stringToInt(instructionString[5]);
      in->in2Idx_ = stringToInt(instructionString[6]);
      in->in3Idx_ = stringToInt(instructionString[7]);
      instructions_.push_back(in);
   }
   instructions_effective_ = instructions_;
   op_counts_.resize(instruction::NUM_OP);
   from_string_ = true;
}

RegisterMachine::~RegisterMachine() {
   for (auto *i : instructions_) delete i;
   for (auto *m : private_memory_) delete m;
   for (auto *m : observation_memory_buff_) delete m;
}

// TODO(skelly): WARNING: confirm this function works as expected.
void RegisterMachine::MarkFeatures(instruction *istr, int in) {
   // Setting input memory pointer is required for MarkIntrons.
   istr->SetInMem(in, observation_memory_buff_[istr->GetInType(in)]);

   features_.clear();
   if (istr->GetInType(in) == MemoryEigen::kScalarType_) {
      features_.insert(istr->GetInIdx(in));
   } else if (istr->GetInType(in) == MemoryEigen::kVectorType_) {
      for (size_t f = istr->GetInIdx(in), row = 0;
           row < istr->GetInMem(in)->memory_size_; row++) {
         // features_.insert(f++ % num_input_);  // toroidal
         features_.insert(f++);
      }
   } else if (istr->GetInType(in) == MemoryEigen::kMatrixType_) {
      for (size_t f = istr->GetInIdx(in), row = 0;
           row < istr->GetInMem(in)->memory_size_; row++) {
         for (size_t col = 0; col < istr->GetInMem(in)->memory_size_; col++) {
            // features_.insert(f++ % num_input_);  // toroidal
            features_.insert(f++);
         }
      }
   }
}

void RegisterMachine::MarkIntrons(
    std::unordered_map<std::string, std::any> &params) {
   // memories_effective keeps track of which memories are effective, i.e. used
   // in the program. memories_effective maps [memory type][index]->true/false.
   auto n_memories = std::any_cast<int>(params["n_memories"]);
   map<int, vector<bool>> memories_effective;
   memories_effective[MemoryEigen::kScalarType_] =
       vector<bool>(n_memories, false);
   memories_effective[MemoryEigen::kVectorType_] =
       vector<bool>(n_memories, false);
   memories_effective[MemoryEigen::kMatrixType_] =
       vector<bool>(n_memories, false);

   // Mark bid output memory
   memories_effective[MemoryEigen::kScalarType_][0] = true;

   // Mark continuous output memory
   if (std::any_cast<int>(params["continuous_output"]) == 1)
      memories_effective[MemoryEigen::kScalarType_][1] = true;
   else if (std::any_cast<int>(params["continuous_output"]) == 2)
      memories_effective[MemoryEigen::kVectorType_][1] = true;
   else if (std::any_cast<int>(params["continuous_output"]) == 3)
      memories_effective[MemoryEigen::kMatrixType_][1] = true;

   // Backward pass to find effective instructions when stateless
   std::vector<instruction *> instructions_effective_stateless;
   for (auto riter = instructions_.rbegin(); riter != instructions_.rend();
        riter++) {
      auto istr = *riter;
      if (memories_effective[istr->GetOutType()][istr->outIdx_ % n_memories]) {
         instructions_effective_stateless.push_back(istr);
         for (int in = 0; in < 2; in++) {
            if (istr->IsMemoryRef(in)) {
               memories_effective[istr->GetInType(in)]
                                 [istr->GetInIdx(in) % n_memories] = true;
            }
         }
      }
   }

   // TODO(skelly): Is this the most efficient method? Currently O(n^2)
   for (size_t t = 0; t < instructions_.size(); t++) {
      instructions_effective_.clear();
      // Count occurance of each op.
      std::fill(op_counts_.begin(), op_counts_.end(), 0);
      for (auto istr : instructions_) {
         if (memories_effective[istr->GetOutType()]
                               [istr->outIdx_ % n_memories] ||
             std::find(instructions_effective_stateless.begin(),
                       instructions_effective_stateless.end(),
                       istr) != instructions_effective_stateless.end()) {
            instructions_effective_.push_back(istr);
            op_counts_[istr->op_]++;
            for (int in = 0; in < 2; in++) {
               if (istr->IsMemoryRef(in)) {
                  memories_effective[istr->GetInType(in)]
                                    [istr->GetInIdx(in) % n_memories] = true;
               } else if (istr->IsObs(in)) {
                  MarkFeatures(istr, in);
               }
            }
         }
      }
   }
}

void RegisterMachine::Mutate(std::unordered_map<std::string, std::any> &params,
                             std::unordered_map<std::string, int> &state,
                             mt19937 &rng, vector<bool> &legal_ops) {
   uniform_real_distribution<> dis_real(0, 1.0);
   bool changed = false;

   while (!changed) {
      // Remove random instruction
      if (instructions_.size() > 1 &&
          dis_real(rng) <
              std::any_cast<double>(params["p_instructions_delete"])) {
         uniform_int_distribution<int> disBid(0, instructions_.size() - 1);
         int i = disBid(rng);
         delete *(instructions_.begin() + i);
         instructions_.erase(instructions_.begin() + i);
         changed = true;
      }

      // Insert a new random instruction
      if ((int)instructions_.size() <
              std::any_cast<int>(params["max_prog_size"]) &&
          dis_real(rng) < std::any_cast<double>(params["p_instructions_add"])) {
         instruction *instr = new instruction(params, rng);
         instr->Mutate(true, legal_ops, observation_buff_size_, rng);
         uniform_int_distribution<int> disBid(0, instructions_.size());
         int i = disBid(rng);
         instructions_.insert(instructions_.begin() + i, instr);
         changed = true;
      }

      // Mutate a randomly selected instruction
      if (dis_real(rng) <
          std::any_cast<double>(params["p_instructions_mutate"])) {
         uniform_int_distribution<int> disBid(0, instructions_.size() - 1);
         instructions_[disBid(rng)]->Mutate(false, legal_ops,
                                            observation_buff_size_, rng);
         changed = true;
      }

      // Mutate constants
      if (use_evolved_const_ && dis_real(rng) <
          std::any_cast<double>(params["p_memory_mu_const"])) {
         for (auto m : private_memory_) {
            m->MutateConstants(rng);
         }
      }

      // Swap positions of two instructions
      if (instructions_.size() > 1 &&
          dis_real(rng) <
              std::any_cast<double>(params["p_instructions_swap"])) {
         uniform_int_distribution<int> disBid(0, instructions_.size() - 1);
         int i = disBid(rng);
         int j;
         do {
            j = disBid(rng);
         } while (i == j);
         std::swap(instructions_[i], instructions_[j]);
         changed = true;
      }

      // // Change observation buff size
      // if (dis_real(rng) <
      //     std::any_cast<double>(params["p_observation_buff_size"])) {
      //   MutateObsBuffSize(std::any_cast<int>(params["max_observation_buff_size"]),
      //                     rng);
      //   changed = true;
      // }

      // Change memory size
      if (dis_real(rng) < std::any_cast<double>(params["p_memory_size"])) {
         MutateMemorySize(params, state, rng);
         changed = true;
      }

      // Change observation index
      if (dis_real(rng) <
          std::any_cast<double>(params["p_observation_index"])) {
         const int max_index = 1000000;  // TODO(skelly): fix magic #
         uniform_int_distribution<int> dis(0, max_index);
         auto prev = obs_index_;
         do {
            obs_index_ = dis(rng);
         } while (obs_index_ == prev);
         changed = true;
      }
   }
}

// This functions currently assumes obs is a vector of state vars
void RegisterMachine::CopyObservationToMemoryBuff(state *obs, size_t mem_t) {
   auto memory_size = private_memory_[0]->memory_size_;
   // TODO(skelly): this can be optimized further
   int n_col = mem_t == MemoryEigen::kVectorType_ ? 1 : memory_size;
   MatrixDynamic mat(memory_size, n_col);

   if (mem_t == MemoryEigen::kVectorType_) {  // copy obs to vector
      int f = obs_index_ % obs->dim_;
      for (size_t row = 0; row < memory_size; row++) {
         mat(row, 0) = obs->stateValueAtIndex(f % obs->dim_);
         f++;
      }
   } else if (mem_t == MemoryEigen::kMatrixType_) {  // copy obs to matrix
      int fm = obs_index_ % obs->dim_;
      for (size_t row = 0; row < memory_size; row++) {
         for (size_t col = 0; col < memory_size; col++) {
            mat(row, col) = obs->stateValueAtIndex(fm % obs->dim_);
            fm++;
         }
      }
   }
   AddToInputMemoryBuff(mat, mem_t);
}

void RegisterMachine::Run(state *obs, int &time_step, const size_t &graph_depth,
                          bool &verbose) {
   // Clear working memory prior to execution, making this program stateless
   if (!stateful_) {
      if (use_evolved_const_) {
         CopyPrivateConstToWorkingMemory();
      } else {
         ClearWorkingMemory();
      }
   }
   bool copied_obs_vec = false;
   bool copied_obs_mat = false;

   for (auto istr : instructions_effective_) {
      istr->out_ = private_memory_[istr->GetOutType()];

      istr->outIdxE_ = istr->outIdx_ % istr->out_->n_memories_;

      for (size_t in = 0; in < 2; in++) {
         // Check if this input is used in the operation.
         if (istr->GetInType(in) != -1) {
            if (istr->IsMemoryRef(in)) {
               istr->SetInMem(in, private_memory_[istr->GetInType(in)]);
               istr->SetInIdxE(
                   in, istr->GetInIdx(in) % istr->GetInMem(in)->n_memories_);

               // Input is a memory ref. Track read time
               istr->GetInMem(in)->read_time_[istr->GetInIdxE(in)] =
                   time_step + (graph_depth / MAX_GRAPH_DEPTH);
            } else {  // Input is an observation reference
               istr->SetInMem(in,
                              observation_memory_buff_[istr->GetInType(in)]);
               istr->SetInIdxE(
                   in, istr->GetInIdx(in) % istr->GetInMem(in)->n_memories_);
               // Copy to obs buff only once.
               if (istr->GetInType(in) == MemoryEigen::kVectorType_ &&
                   !copied_obs_vec) {
                  CopyObservationToMemoryBuff(obs, MemoryEigen::kVectorType_);
                  copied_obs_vec = true;
               } else if (istr->GetInType(in) == MemoryEigen::kMatrixType_ &&
                          !copied_obs_mat) {
                  CopyObservationToMemoryBuff(obs, MemoryEigen::kMatrixType_);
                  copied_obs_mat = true;
               }
            }
            // This copies scalar input data to temporary scalar variables
            if (istr->GetInType(in) == MemoryEigen::kScalarType_) {
               istr->SetupScalarIn(in, obs);
            }
         }
      }
      // Track write times for temporal memory
      istr->out_->write_time_[istr->outIdxE_] =
          time_step + (graph_depth / MAX_GRAPH_DEPTH);
      istr->exec(verbose);  // Execute instruction
   }
   bid_val_ =
       private_memory_[MemoryEigen::kScalarType_]->working_memory_[0](0, 0);
}

void RegisterMachine::SetupMemory(std::unordered_map<std::string, int> &state,
                                  int n_memories, int memory_size) {
   for (size_t mem_t = 0; mem_t < MemoryEigen::kNumMemoryType_; mem_t++) {
      private_memory_.push_back(
          new MemoryEigen(mem_t, n_memories, memory_size));
         private_memory_.back()->RandomizeConst();
      observation_memory_buff_.push_back(
          new MemoryEigen(mem_t, observation_buff_size_, memory_size));
   }
}

void RegisterMachine::ResizeMemory(
    std::unordered_map<std::string, std::any> &params,
    std::unordered_map<std::string, int> &state, int new_memory_size) {
   for (auto *m : private_memory_) {
      m->memory_size_ = new_memory_size;
      m->n_memories_ = std::any_cast<int>(params["n_memories"]);
      m->ResizeMemory();
         m->RandomizeConst();
   }
   for (auto *m : observation_memory_buff_) {
      m->memory_size_ = new_memory_size;
      m->n_memories_ = std::any_cast<int>(params["n_memories"]);
      m->ResizeMemory();
   }
}

// void RegisterMachine::MutateObsBuffSize(size_t max_observation_buff_size,
//                                         mt19937 &rng) {
//   std::uniform_int_distribution<> dis(1, max_observation_buff_size - 1);
//   auto prev = observation_buff_size_;
//   do {
//     observation_buff_size_ = dis(rng);
//   } while (observation_buff_size_ == prev);
//   // ResizeMemory();
// }

void RegisterMachine::MutateMemorySize(
    std::unordered_map<std::string, std::any> &params,
    std::unordered_map<std::string, int> &state, mt19937 &rng) {
   std::uniform_int_distribution<> dis(
       std::any_cast<int>(params["min_memory_size"]),
       std::any_cast<int>(params["max_memory_size"]));
   size_t old_size = private_memory_[MemoryEigen::kScalarType_]->memory_size_;
   size_t new_size;
   do {
      new_size = dis(rng);
   } while (new_size == old_size);
   ResizeMemory(params, state, new_size);
}
