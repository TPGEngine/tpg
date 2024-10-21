#include "RegisterMachine.h"

string RegisterMachine::checkpoint(bool effective_only) {
  ostringstream oss;
  oss << "RegisterMachine:" << id_ << ":" << gtime_ << ":" << action_ << ":"
      << stateful_ << ":" << nrefs_ << ":" << observation_buff_size_ << ":"
      << obs_index_ << ":" << memory_size_;
  auto prog = effective_only ? bidEffective_ : bid_;
  for (auto istr : prog) oss << ":" << istr->checkpoint();
  oss << endl;
  // CheckMemorySizes(8);
  return oss.str();
}

// Create arbitrary RegisterMachine
RegisterMachine::RegisterMachine(
    long gtime, long action, std::unordered_map<std::string, std::any> &params,
    long id, mt19937 &rng, std::vector<bool> &legalOps) {
  action_ = action;
  stateful_ = std::any_cast<int>(params["stateful"]);
  gtime_ = gtime;
  id_ = id;
  key_ = 0;
  nrefs_ = 0;
  observation_buff_size_ = std::any_cast<int>(params["observation_buff_size"]);
  memory_size_ = std::any_cast<int>(params["memory_size"]);
  const int max_index = 100;
  uniform_int_distribution<int> dis(0, max_index);
  obs_index_ = dis(rng);
  // uniform_real_distribution<double> disR(0.0, 1.0);
  uniform_int_distribution<int> disP(
      1, std::any_cast<int>(params["max_initial_prog_size"]));
  int prog_size = disP(rng);
  for (int i = 0; i < prog_size; i++) {
    auto in = new instruction(params, rng);
    in->Mutate(true, legalOps, observation_buff_size_, rng);
    bid_.push_back(in);
  }
  op_counts_.resize(instruction::NUM_OP);
  SetupMemory(std::any_cast<int>(params["memory_indices"]));
  // CheckMemorySizes(std::any_cast<int>(params["memory_indices"]));
}

// Create RegisterMachine from another RegisterMachine
RegisterMachine::RegisterMachine(
    long gtime, RegisterMachine &plr,
    std::unordered_map<std::string, std::any> &params, long id) {
  action_ = plr.action();
  gtime_ = gtime;
  id_ = id;
  key_ = plr.key();
  obs_index_ = plr.obs_index_;
  bid_val_ = -(numeric_limits<double>::max());
  nrefs_ = 0;
  stateful_ = plr.stateful_;
  observation_buff_size_ = plr.observation_buff_size_;
  memory_size_ = plr.memory_size_;

  for (auto initer = plr.bid_.begin(); initer != plr.bid_.end(); initer++)
    bid_.push_back(new instruction(**initer));
  op_counts_.resize(instruction::NUM_OP);
  SetupMemory(std::any_cast<int>(params["memory_indices"]));
}

// Create RegisterMachine from checkpoint file
RegisterMachine::RegisterMachine(
    long gtime, long action, int stateful,
    std::unordered_map<std::string, std::any> &params, long id, long nrefs,
    int observation_buff_size, int memory_size,
    std::vector<instruction *> bid) {
  action_ = action;
  bid_ = bidEffective_ = bid;
  gtime_ = gtime;
  id_ = id;
  key_ = 0;
  nrefs_ = nrefs;
  stateful_ = stateful > 0 ? true : false;
  observation_buff_size_ = observation_buff_size;
  memory_size_ = memory_size;
  op_counts_.resize(instruction::NUM_OP);
  SetupMemory(std::any_cast<int>(params["memory_indices"]));
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
  // Meff keeps track of which memories are effective, i.e. used in the program.
  // Meff maps [memory type][index]->true/false.
  auto memory_indices = std::any_cast<int>(params["memory_indices"]);
  map<int, vector<bool> > Meff;
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
          Meff[istr->GetInType(in)][istr->GetInIdx(in) % memory_indices] = true;
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
            Meff[istr->GetInType(in)][istr->GetInIdx(in) % memory_indices] =
                true;
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
    /* Remove random instruction. */
    if (bid_.size() > 1 &&
        dis_real(rng) < std::any_cast<double>(params["p_bid_delete"])) {
      uniform_int_distribution<int> disBid(0, bid_.size() - 1);
      int i = disBid(rng);
      delete *(bid_.begin() + i);
      bid_.erase(bid_.begin() + i);
      changed = true;
    }

    /* Insert random instruction. */
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

    /* Mutate a random instruction. */
    if (dis_real(rng) < std::any_cast<double>(params["p_bid_mutate"])) {
      uniform_int_distribution<int> disBid(0, bid_.size() - 1);
      bid_[disBid(rng)]->Mutate(false, legalOps, observation_buff_size_, rng);
      changed = true;
    }

    // /* Add noise to constants */
    // if (params.find("p_bid_mu_const") != params.end() &&
    //     disR(rng) < std::any_cast<double>(params["p_bid_mu_const"])) {
    //   for (auto m : sharedMemory_) {
    //     m->NoiseToConst(rng,
    //                     std::any_cast<double>(params["bid_mu_const_stddev"]));
    //   }
    // }

    /* Swap positions of two instructions. */
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

    // // Change observation buff size.
    // if (dis_real(rng) <
    //     std::any_cast<double>(params["p_observation_buff_size"])) {
    //   MutateObsBuffSize(std::any_cast<int>(params["max_observation_buff_size"]),
    //                     rng);
    //   changed = true;
    // }

    // Change memory size.
    if (dis_real(rng) < std::any_cast<double>(params["p_memory_size"])) {
      MutateMemorySize(params, rng);
      changed = true;
    }

    // Change observation index.
    if (dis_real(rng) < std::any_cast<double>(params["p_observation_index"])) {
      const int max_index = 100;
      uniform_int_distribution<int> dis(0, max_index);
      obs_index_ = dis(rng);
    }

    // for (auto istr : bid_) istr->BoundMemoryIndices(observation_buff_size_);
  }

  // CheckMemorySizes(std::any_cast<int>(params["memory_indices"]));
}

// TODO(skelly): This functions currently assumes obs is a vector of state vars
void RegisterMachine::CopyObservationToMemoryBuff(state *obs,
                                                  size_t memory_type) {
  if (memory_type == memoryEigen::VECTOR_TYPE) {
    int f = obs_index_ % obs->dim_;
    for (int row = 0; row < memory_size_; row++) {
      observation_memory_buff_[memoryEigen::VECTOR_TYPE]->working_memory_[0](
          row, 0) = obs->stateValueAtIndex(f % obs->dim_);
      f++;
    }
  } else if (memory_type == memoryEigen::MATRIX_TYPE) {
    int fm = obs_index_ % obs->dim_;
    for (int row = 0; row < memory_size_; row++) {
      for (int col = 0; col < memory_size_; col++) {
        observation_memory_buff_[memoryEigen::MATRIX_TYPE]->working_memory_[0](
            row, col) = obs->stateValueAtIndex(fm % obs->dim_);
        fm++;
      }
    }
  }
}

double RegisterMachine::Run(state *obs, int &time_step,
                            const size_t &graph_depth, bool &verbose) {
                              
  // Clear working memory prior to execution, making this program stateless
  if (!stateful_) ClearWorking();

  bool copied_obs_vec = false;
  bool copied_obs_mat = false;

  for (auto istr : bidEffective_) {
    istr->out_ = privateMemory_[istr->GetOutType()];

    // memoryIndices_ and memory_size_ can be dynamic
    // Must do mods here at runtime.
    istr->outIdxE_ = istr->outIdx_ % istr->out_->memoryIndices_;
    // TODO(skelly): these are unused
    // istr->SetInIdxE(2, istr->in2Idx_ % memory_size_);
    // istr->SetInIdxE(3, istr->in3Idx_ % memory_size_);

    for (size_t in = 0; in < 2; in++) {
      // Check if this input is used in the operation.
      if (istr->GetInType(in) != memoryEigen::NA_TYPE) {
        if (istr->IsMemoryRef(in)) {
          istr->SetInMem(in, privateMemory_[istr->GetInType(in)]);

          // memoryIndices_ and memory_size_ can be dynamic, so do mods here.
          istr->SetInIdxE(
              in, istr->GetInIdx(in) % istr->GetInMem(in)->memoryIndices_);

          // Input is a memory ref. Track read time for temporal memory.
          istr->GetInMem(in)->getReadTimeE()(istr->GetInIdxE(in), 0) =
              time_step + (graph_depth / MAX_GRAPH_DEPTH);
        } else {  // Input is an observation reference.
          istr->SetInMem(in, observation_memory_buff_[istr->GetInType(in)]);
          // memoryIndices_ and memory_size_ can be dynamic, so do mods here.
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

        // This copies input data to temporary scalar input variables.
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

void RegisterMachine::SetupMemory(int memory_indices) {
  for (int mem_t = 0; mem_t < memoryEigen::NUM_MEMORY_TYPES; mem_t++) {
    privateMemory_.push_back(
        new memoryEigen(-1, mem_t, memory_indices, memory_size_));
    observation_memory_buff_.push_back(
        new memoryEigen(-1, mem_t, observation_buff_size_, memory_size_));
  }
  for (auto istr : bid_) istr->memory_size_ = memory_size_;
  for (auto istr : bidEffective_) istr->memory_size_ = memory_size_;
  // CheckMemorySizes(8);
}

// void RegisterMachine::ResizeMemory(int new_size) {
//   // for (auto memory : observation_memory_buff_) {
//   //   // memory->memoryIndices_ = observation_buff_size_;
//   //   memory->memory_size_ = new_size;
//   //   memory->ResizeMemory();
//   // }
//   for (size_t i = 0; i < observation_memory_buff_.size(); i++) {
//     observation_memory_buff_[i]->memory_size_ = new_size;
//     observation_memory_buff_[i]->ResizeMemory();
//   }
//   // for (auto memory : privateMemory_) {
//   //   memory->memory_size_ = new_size;
//   //   memory->ResizeMemory();
//   // }
//   for (size_t i = 0; i < privateMemory_.size(); i++) {
//     privateMemory_[i]->memory_size_ = new_size;
//     privateMemory_[i]->ResizeMemory();
//   }
//   for (auto istr : bid_) istr->memory_size_ = memory_size_;
// }

void RegisterMachine::ResizeMemory(int memory_indices) {
  for (size_t i = 0; i < observation_memory_buff_.size(); i++) {
    delete observation_memory_buff_[i];
  }
  observation_memory_buff_.clear();

  for (size_t i = 0; i < privateMemory_.size(); i++) {
    delete privateMemory_[i];
  }
  privateMemory_.clear();

  SetupMemory(memory_indices);
  // CheckMemorySizes(8);
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
  // (void)params;
  // (void)rng;
  std::uniform_int_distribution<> dis(
      std::any_cast<int>(params["min_memory_size"]),
      std::any_cast<int>(params["max_memory_size"]));
  auto prev = memory_size_;
  do {
    memory_size_ = dis(rng);
  } while (memory_size_ == prev);
  ResizeMemory(std::any_cast<int>(params["memory_indices"]));
  for (auto istr : bid_) istr->memory_size_ = memory_size_;
  for (auto istr : bidEffective_) istr->memory_size_ = memory_size_;
  // CheckMemorySizes(std::any_cast<int>(params["memory_indices"]));
}
