#include "RegisterMachine.h"

/******************************************************************************/
string RegisterMachine::checkpoint(bool effective_only) {
   ostringstream oss;
   oss << "RegisterMachine:" << id_ << ":" << gtime_ << ":" << action_ << ":"
       << stateful_ << ":" << nrefs_ << ":" << observation_buff_size_ << ":"
       << obs_index_ << ":" << memory_size_;
   for (auto &i : private_memory_ids_) {
      oss << ":" << i;
   }
   auto prog = effective_only ? bidEffective_ : bid_;
   for (auto istr : prog) {
      oss << ":" << istr->checkpoint();
   }
   oss << endl;
   return oss.str();
}

/******************************************************************************/
// Create arbitrary RegisterMachine
RegisterMachine::RegisterMachine(
    long action, std::unordered_map<std::string, std::any> &params,
    std::unordered_map<std::string, int> &state, mt19937 &rng,
    std::vector<bool> &legalOps) {
   action_ = action;
   stateful_ = std::any_cast<int>(params["stateful"]);
   gtime_ = state["t_current"];
   id_ = state["program_count"]++;
   key_ = 0;
   nrefs_ = 0;
   observation_buff_size_ = std::any_cast<int>(params["observation_buff_size"]);
   memory_size_ = std::any_cast<int>(params["memory_size"]);
   const int max_index = 100;
   uniform_int_distribution<int> dis(0, max_index);
   obs_index_ = dis(rng);
   uniform_int_distribution<int> disP(
       1, std::any_cast<int>(params["max_initial_prog_size"]));
   int prog_size = disP(rng);
   for (int i = 0; i < prog_size; i++) {
      auto in = new instruction(params, rng);
      in->Mutate(true, legalOps, observation_buff_size_, rng);
      bid_.push_back(in);
   }
   op_counts_.resize(instruction::NUM_OP);
   if (!isEqual(std::any_cast<double>(params["p_bid_mu_const"]), 0.0)) {
      use_evolved_const_ = true;
   }
   SetupMemory(params, state);
}

// Create RegisterMachine from another RegisterMachine
RegisterMachine::RegisterMachine(
    RegisterMachine &plr, std::unordered_map<std::string, std::any> &params,
    std::unordered_map<std::string, int> &state) {
   action_ = plr.action();
   gtime_ = state["t_current"];
   id_ = state["program_count"]++;
   key_ = plr.key();
   obs_index_ = plr.obs_index_;
   bid_val_ = -(numeric_limits<double>::max());
   nrefs_ = 0;
   stateful_ = plr.stateful_;
   observation_buff_size_ = plr.observation_buff_size_;
   memory_size_ = plr.memory_size_;
   use_evolved_const_ = plr.use_evolved_const_;
   for (auto initer = plr.bid_.begin(); initer != plr.bid_.end(); initer++)
      bid_.push_back(new instruction(**initer));
   op_counts_.resize(instruction::NUM_OP);
   SetupMemory(params, state);
   if (use_evolved_const_) {
      CopyEvolvedConstants(plr);
   }
}

// Create RegisterMachine from checkpoint
RegisterMachine::RegisterMachine(
    std::vector<std::string> outcomeFields,
    std::vector<std::map<long, memoryEigen *>> &memory_maps,
    std::unordered_map<std::string, std::any> &params, mt19937 &rng) {
   int f = 1;
   id_ = atoi(outcomeFields[f++].c_str());
   gtime_ = atoi(outcomeFields[f++].c_str());
   action_ = atoi(outcomeFields[f++].c_str());
   stateful_ = atoi(outcomeFields[f++].c_str());
   nrefs_ = atoi(outcomeFields[f++].c_str());
   observation_buff_size_ = atoi(outcomeFields[f++].c_str());
   obs_index_ = atoi(outcomeFields[f++].c_str());
   memory_size_ = atoi(outcomeFields[f++].c_str());
   // SetupMemry() but from existing memory pointers
   for (int mem_t = 0; mem_t < memoryEigen::NUM_MEMORY_TYPES; mem_t++) {
      long id = atoi(outcomeFields[f++].c_str());
      privateMemory_.push_back(memory_maps[mem_t][id]);
      private_memory_ids_.push_back(id);
      observation_memory_buff_.push_back(
          new memoryEigen(-1, mem_t, observation_buff_size_, memory_size_));
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
      in->memory_size_ = stringToInt(instructionString[8]);
      bid_.push_back(in);
   }
   bidEffective_ = bid_;
   key_ = 0;  // TODO(skelly): check needed
   op_counts_.resize(instruction::NUM_OP);
}

RegisterMachine::~RegisterMachine() {
   for (auto instr : bid_) delete instr;
   bid_.clear();
   for (size_t m = 0; m < privateMemory_.size(); m++) delete privateMemory_[m];
   privateMemory_.clear();
   for (auto memory : observation_memory_buff_) delete memory;
   observation_memory_buff_.clear();
}

// TODO(skelly): WARNING: confirm this function works as expected.
void RegisterMachine::MarkFeatures(instruction *istr, int in) {
   // Setting input memory pointer is required for MarkIntrons.
   istr->SetInMem(in, observation_memory_buff_[istr->GetInType(in)]);

   features_.clear();
   if (istr->GetInType(in) == memoryEigen::SCALAR_TYPE) {
      features_.insert(istr->GetInIdx(in));
   } else if (istr->GetInType(in) == memoryEigen::VECTOR_TYPE) {
      for (size_t f = istr->GetInIdx(in), row = 0;
           row < istr->GetInMem(in)->memory_size_; row++) {
         // features_.insert(f++ % num_input_);  // toroidal
         features_.insert(f++);
      }
   } else if (istr->GetInType(in) == memoryEigen::MATRIX_TYPE) {
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
   // Meff keeps track of which memories are effective, i.e. used in the
   // program. Meff maps [memory type][index]->true/false.
   auto memory_indices = std::any_cast<int>(params["memory_indices"]);
   map<int, vector<bool>> Meff;
   Meff[memoryEigen::SCALAR_TYPE] = vector<bool>(memory_indices, false);
   Meff[memoryEigen::VECTOR_TYPE] = vector<bool>(memory_indices, false);
   Meff[memoryEigen::MATRIX_TYPE] = vector<bool>(memory_indices, false);

   // Mark bid output memory.
   Meff[memoryEigen::SCALAR_TYPE][0] = true;

   // Mark continuous output memory.
   if (std::any_cast<int>(params["continuous_output"]) == 1)
      Meff[memoryEigen::SCALAR_TYPE][1] = true;
   else if (std::any_cast<int>(params["continuous_output"]) == 2)
      Meff[memoryEigen::VECTOR_TYPE][1] = true;
   else if (std::any_cast<int>(params["continuous_output"]) == 3)
      Meff[memoryEigen::MATRIX_TYPE][1] = true;

   // Backward pass to find effective instructions when stateless
   std::vector<instruction *> bid_effective_stateless;
   for (auto riter = bid_.rbegin(); riter != bid_.rend(); riter++) {
      auto istr = *riter;
      if (Meff[istr->GetOutType()][istr->outIdx_ % memory_indices]) {
         bid_effective_stateless.push_back(istr);
         for (int in = 0; in < 2; in++) {
            if (istr->IsMemoryRef(in)) {
               Meff[istr->GetInType(in)][istr->GetInIdx(in) % memory_indices] =
                   true;
            }
         }
      }
   }

   // TODO(skelly): Is this the most efficient method? Currently O(n^2)
   for (size_t t = 0; t < bid_.size(); t++) {
      bidEffective_.clear();
      // Count occurance of each op.
      std::fill(op_counts_.begin(), op_counts_.end(), 0);
      for (auto istr : bid_) {
         if (Meff[istr->GetOutType()][istr->outIdx_ % memory_indices] ||
             std::find(bid_effective_stateless.begin(),
                       bid_effective_stateless.end(),
                       istr) != bid_effective_stateless.end()) {
            bidEffective_.push_back(istr);
            op_counts_[istr->op_]++;
            for (int in = 0; in < 2; in++) {
               if (istr->IsMemoryRef(in)) {
                  Meff[istr->GetInType(in)]
                      [istr->GetInIdx(in) % memory_indices] = true;
               } else if (istr->IsObs(in)) {
                  MarkFeatures(istr, in);
               }
            }
         }
      }
   }
}

void RegisterMachine::Mutate(std::unordered_map<std::string, std::any> &params,
                             mt19937 &rng, vector<bool> &legalOps) {
   uniform_real_distribution<> dis_real(0, 1.0);
   bool changed = false;

   while (!changed) {
      // Remove random instruction
      if (bid_.size() > 1 &&
          dis_real(rng) < std::any_cast<double>(params["p_bid_delete"])) {
         uniform_int_distribution<int> disBid(0, bid_.size() - 1);
         int i = disBid(rng);
         delete *(bid_.begin() + i);
         bid_.erase(bid_.begin() + i);
         changed = true;
      }

      // Insert random instruction
      if ((int)bid_.size() < std::any_cast<int>(params["max_prog_size"]) &&
          dis_real(rng) < std::any_cast<double>(params["p_bid_add"])) {
         instruction *instr = new instruction(params, rng);
         instr->memory_size_ = memory_size_;
         instr->Mutate(true, legalOps, observation_buff_size_, rng);
         uniform_int_distribution<int> disBid(0, bid_.size());
         int i = disBid(rng);
         bid_.insert(bid_.begin() + i, instr);
         changed = true;
      }

      // Mutate a random instruction
      if (dis_real(rng) < std::any_cast<double>(params["p_bid_mutate"])) {
         uniform_int_distribution<int> disBid(0, bid_.size() - 1);
         bid_[disBid(rng)]->Mutate(false, legalOps, observation_buff_size_,
                                   rng);
         changed = true;
      }

      // Add noise to constants
      if (dis_real(rng) < std::any_cast<double>(params["p_bid_mu_const"])) {
         for (auto m : privateMemory_) {
            m->NoiseToConst(
                rng, std::any_cast<double>(params["bid_mu_const_stddev"]));
         }
      }

      // Swap positions of two instructions
      if (bid_.size() > 1 &&
          dis_real(rng) < std::any_cast<double>(params["p_bid_swap"])) {
         uniform_int_distribution<int> disBid(0, bid_.size() - 1);
         int i = disBid(rng);
         int j;
         do {
            j = disBid(rng);
         } while (i == j);
         std::swap(bid_[i], bid_[j]);
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
         MutateMemorySize(params, rng);
         changed = true;
      }

      // Change observation index
      if (dis_real(rng) <
          std::any_cast<double>(params["p_observation_index"])) {
         const int max_index = 100;
         uniform_int_distribution<int> dis(0, max_index);
         obs_index_ = dis(rng);
      }
   }
}

// TODO(skelly): This functions currently assumes obs is a vector of state vars
void RegisterMachine::CopyObservationToMemoryBuff(state *obs, size_t mem_t) {
   //TODO(skelly): this can be optimized further
   int n_col = mem_t == memoryEigen::VECTOR_TYPE ? 1 : memory_size_;
   Matrix<double, Dynamic, Dynamic> mat(memory_size_, n_col);                                            
   
   if (mem_t == memoryEigen::VECTOR_TYPE) {  // copy obs to vector
      int f = obs_index_ % obs->dim_;
      for (int row = 0; row < memory_size_; row++) {
         mat(row, 0) = obs->stateValueAtIndex(f % obs->dim_);
         f++;
      }
   } else if (mem_t == memoryEigen::MATRIX_TYPE) {  // copy obs to matrix
      int fm = obs_index_ % obs->dim_;
      for (int row = 0; row < memory_size_; row++) {
         for (int col = 0; col < memory_size_; col++) {
            mat(row, col) = obs->stateValueAtIndex(fm % obs->dim_);
            fm++;
         }
      }
   }
   AddToInputMemoryBuff(mat, mem_t);
}

double RegisterMachine::Run(state *obs, int &time_step,
                            const size_t &graph_depth, bool &verbose) {
   // Clear working memory prior to execution, making this program stateless
   if (!stateful_) {
      if (use_evolved_const_) {
         CopyPrivateConstToWorking();
      } else {
         ClearWorking();
      }
   }
   bool copied_obs_vec = false;
   bool copied_obs_mat = false;

   for (auto istr : bidEffective_) {
      istr->out_ = privateMemory_[istr->GetOutType()];

      istr->outIdxE_ = istr->outIdx_ % istr->out_->memoryIndices_;
      // TODO(skelly): these are unused
      // istr->SetInIdxE(2, istr->in2Idx_ % memory_size_);
      // istr->SetInIdxE(3, istr->in3Idx_ % memory_size_);

      for (size_t in = 0; in < 2; in++) {
         // Check if this input is used in the operation.
         if (istr->GetInType(in) != memoryEigen::NA_TYPE) {
            if (istr->IsMemoryRef(in)) {
               istr->SetInMem(in, privateMemory_[istr->GetInType(in)]);
               istr->SetInIdxE(
                   in, istr->GetInIdx(in) % istr->GetInMem(in)->memoryIndices_);

               // Input is a memory ref. Track read time 
               istr->GetInMem(in)->getReadTimeE()(istr->GetInIdxE(in), 0) =
                   time_step + (graph_depth / MAX_GRAPH_DEPTH);
            } else {  // Input is an observation reference
               istr->SetInMem(in,
                              observation_memory_buff_[istr->GetInType(in)]);
               istr->SetInIdxE(
                   in, istr->GetInIdx(in) % istr->GetInMem(in)->memoryIndices_);
               // Copy to obs buff only once.
               if (istr->GetInType(in) == memoryEigen::VECTOR_TYPE &&
                   !copied_obs_vec) {
                  CopyObservationToMemoryBuff(obs, memoryEigen::VECTOR_TYPE);
                  copied_obs_vec = true;
               } else if (istr->GetInType(in) == memoryEigen::MATRIX_TYPE &&
                          !copied_obs_mat) {
                  CopyObservationToMemoryBuff(obs, memoryEigen::MATRIX_TYPE);
                  copied_obs_mat = true;
               }
            }
            // This copies scalar input data to temporary scalar variables
            if (istr->GetInType(in) == memoryEigen::SCALAR_TYPE) {
               istr->SetupScalarIn(in, obs);
            }
         }
      }
      // Track write times for temporal memory.
      istr->out_->getWriteTimeE()(istr->outIdxE_, 0) =
          time_step + (graph_depth / MAX_GRAPH_DEPTH);
      istr->exec(verbose);  // Execute instruction
   }
   // Return bid value.
   return privateMemory_[memoryEigen::SCALAR_TYPE]->working_memory_[0](0, 0);
}

void RegisterMachine::SetupMemory(
    std::unordered_map<std::string, std::any> &params,
    std::unordered_map<std::string, int> &state) {
   for (int mem_t = 0; mem_t < memoryEigen::NUM_MEMORY_TYPES; mem_t++) {
      long id = state["memory_count"]++;
      privateMemory_.push_back(new memoryEigen(
          id, mem_t, std::any_cast<int>(params["memory_indices"]),
          memory_size_));
      private_memory_ids_.push_back(id);
      if (use_evolved_const_) {
         privateMemory_.back()->RandomizeConst();
      }
      observation_memory_buff_.push_back(
          new memoryEigen(-1, mem_t, observation_buff_size_, memory_size_));
   }
   for (auto istr : bid_) istr->memory_size_ = memory_size_;
   for (auto istr : bidEffective_) istr->memory_size_ = memory_size_;
}

void RegisterMachine::ResizeMemory(
    std::unordered_map<std::string, std::any> &params) {
   for (auto m : privateMemory_) {
      m->memory_size_ = memory_size_;
      m->memoryIndices_ = std::any_cast<int>(params["memory_indices"]);
      m->ResizeMemory();
      if (use_evolved_const_) {
         m->RandomizeConst();
      }
   }
   for (auto m : observation_memory_buff_) {
      m->memory_size_ = memory_size_;
      m->memoryIndices_ = std::any_cast<int>(params["memory_indices"]);
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
    std::unordered_map<std::string, std::any> &params, mt19937 &rng) {
   std::uniform_int_distribution<> dis(
       std::any_cast<int>(params["min_memory_size"]),
       std::any_cast<int>(params["max_memory_size"]));
   auto prev = memory_size_;
   do {
      memory_size_ = dis(rng);
   } while (memory_size_ == prev);
   ResizeMemory(params);
   for (auto istr : bid_) istr->memory_size_ = memory_size_;
   for (auto istr : bidEffective_) istr->memory_size_ = memory_size_;
}
