#ifndef RegisterMachine_h
#define RegisterMachine_h
#include <any>
#include <bitset>
#include <random>

#include "instruction.h"
#include "memoryEigen.h"
#include "program.h"

struct instructionDecoded;

class RegisterMachine : public program {
 public:
  // Bid program, a list of instructions
  std::vector<instruction *> bid_;
  std::vector<instruction *> bidEffective_;
  // inline void AddToInputMemoryBuff(Matrix<double, Dynamic, Dynamic> &mat,
  //                                  int mem_t) {
  //   observation_memory_buff_[mem_t]->working_memory_.push_front(mat);
  //   observation_memory_buff_[mem_t]->working_memory_.pop_back();
  // }
  void CopyObservationToMemoryBuff(state *obs, size_t memory_type);
  void MarkFeatures(instruction *istr, int in);
  void MarkIntrons(std::unordered_map<std::string, std::any> &params_);
  double Run(state *, int &time_step, const size_t &graph_depth, bool &verbose);
  std::string checkpoint(bool);
  // Create arbitrary RegisterMachine
  RegisterMachine(long, long, std::unordered_map<std::string, std::any> &, long,
                  mt19937 &, std::vector<bool> &);
  // Create RegisterMachine from another RegisterMachine
  RegisterMachine(long, RegisterMachine &,
                  std::unordered_map<std::string, std::any> &, long);
  // Create RegisterMachine from checkpoint file
  RegisterMachine(long, long, int, std::unordered_map<std::string, std::any> &,
                  long, long, int, int, std::vector<instruction *>);
  ~RegisterMachine();
  // Mutate bid
  void Mutate(std::unordered_map<std::string, std::any> &, mt19937 &, std::vector<bool> &);
  void SetupMemory(int memory_indices);
  // void MutateObsBuffSize(size_t max_observation_buff_size, mt19937& rng);
  void MutateMemorySize(std::unordered_map<std::string, std::any> &, mt19937 &);
  // void ResizeMemory(int new_size);
  void ResizeMemory(int memory_indices);
  inline int Size() { return bid_.size(); }
  inline int SizeEffective() { return bidEffective_.size(); }

  inline void CheckMemorySizes(int memory_indices) {
    std::string error_message = "memory_size_error";

    for (int i = 0; i < memory_indices; i++) {
      if (privateMemory_[memoryEigen::VECTOR_TYPE]->working_memory_[i].rows() !=
          memory_size_)
        die(__FILE__, __FUNCTION__, __LINE__, error_message.c_str());
    }

    for (int i = 0; i < memory_indices; i++) {
      if (privateMemory_[memoryEigen::MATRIX_TYPE]->working_memory_[i].rows() !=
              memory_size_ ||
          privateMemory_[memoryEigen::MATRIX_TYPE]->working_memory_[i].cols() !=
              memory_size_)
        die(__FILE__, __FUNCTION__, __LINE__, error_message.c_str());
    }

    int i = 0;
    if (observation_memory_buff_[memoryEigen::VECTOR_TYPE]
            ->working_memory_[i]
            .rows() != memory_size_)
      die(__FILE__, __FUNCTION__, __LINE__, error_message.c_str());

    if (observation_memory_buff_[memoryEigen::MATRIX_TYPE]
                ->working_memory_[i]
                .rows() != memory_size_ ||
        observation_memory_buff_[memoryEigen::MATRIX_TYPE]
                ->working_memory_[i]
                .cols() != memory_size_)
      die(__FILE__, __FUNCTION__, __LINE__, error_message.c_str());

    for (auto istr : bid_)
      if (istr->memory_size_ != memory_size_)
        die(__FILE__, __FUNCTION__, __LINE__, error_message.c_str());
    for (auto istr : bidEffective_)
      if (istr->memory_size_ != memory_size_)
        die(__FILE__, __FUNCTION__, __LINE__, error_message.c_str());
  }
};

#endif
