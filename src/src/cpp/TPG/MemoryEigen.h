#ifndef MemoryEigen_h
#define MemoryEigen_h

#include <Eigen/Dense>
#include <deque>
#include <random>
#include <sstream>
#include <string>

typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> MatrixDynamic;

class MemoryEigen {
  public:
   static inline constexpr size_t kScalarType_ = 0;
   static inline constexpr size_t kVectorType_ = 1;
   static inline constexpr size_t kMatrixType_ = 2;
   static inline constexpr size_t kNumMemoryType_ = 3;

   // long id_;

   // Memory type (kScalarType_, kVectorType_, or kMatrixType_)
   size_t type_;

   // Number of memories (indices) of the given type
   size_t n_memories_;

   // Dimensionality of this memory, depending on its type
   // kScalayType: (1,1)
   // kVectorType_: (memory_size, 1)
   // kMatrixType_: (memory_size_, memory_size_)
   size_t memory_size_;

   // Working memory used for linear program execution
   std::deque<MatrixDynamic> working_memory_;

   // Constant memory to store evolved contants
   // Always same size as working memory
   std::vector<MatrixDynamic> const_memory_;

   // Store latest read time of each memory index
   std::vector<double> read_time_;

   // Store latest write time of each memory index
   std::vector<double> write_time_;

   // Construct random MemoryEigen
   MemoryEigen(int type, size_t n_indices, size_t memory_size)
       : type_(type),
         n_memories_(n_indices),
         memory_size_(memory_size) {
      ResizeMemory();
      ClearWorking();
      RandomizeConst();
      ClearReadTime();
      ClearWriteTime();
   }

   // Copy constructor
   MemoryEigen(MemoryEigen &m) {
      type_ = m.type_;
      n_memories_ = m.n_memories_;
      memory_size_ = m.memory_size_;
      ResizeMemory();
      ClearWorking();
      ClearReadTime();
      ClearWriteTime();
      for (size_t i = 0; i < const_memory_.size(); i++) {
         const_memory_[i] = m.const_memory_[i];
      }
   }

   // Copy assignment operator
   MemoryEigen& operator=(MemoryEigen &m) {
      return *this;
   }

   // Construct MemoryEigen from strings split into outcome_fields vector
   MemoryEigen(std::vector<std::string> &outcome_fields) {
      size_t i = 2;  // Skip program id
      // id_ = std::atoi(outcome_fields[i++].c_str());
      type_ = std::atoi(outcome_fields[i++].c_str());
      n_memories_ = std::atoi(outcome_fields[i++].c_str());
      memory_size_ = std::atoi(outcome_fields[i++].c_str());

      // size_t expected_size;
      // if (type_ == 0)
      //    expected_size = 5 + 8;
      // else if (type_ == 1)
      //    expected_size = 5 + (8 * memory_size_);
      // else
      //    expected_size = 5 + (8 * (memory_size_ * memory_size_));
      // if (outcome_fields.size() != expected_size) {
      //    cerr << "dbg memory_size_ " << memory_size_ << " sz "
      //         << outcome_fields.size() << " ex " << expected_size << endl;
      //    cerr << "str " << VectorToString(outcome_fields) << endl;
      //    die(__FILE__, __FUNCTION__, __LINE__, "Bad memory size.");
      // }
      ResizeMemory();
      // Read in evolved constants
      for (size_t idx = 0; idx < n_memories_; idx++) {
         if (type_ == MemoryEigen::kScalarType_) {
            const_memory_[idx](0, 0) = std::stod(outcome_fields[i++].c_str());
         } else if (type_ == MemoryEigen::kVectorType_) {
            for (size_t r = 0; r < memory_size_; r++) {
               const_memory_[idx](r, 0) =
                   std::stod(outcome_fields[i++].c_str());
            }
         } else if (type_ == MemoryEigen::kMatrixType_) {
            for (size_t r = 0; r < memory_size_; r++)
               for (size_t c = 0; c < memory_size_; c++)
                  const_memory_[idx](r, c) =
                      std::stod(outcome_fields[i++].c_str());
         }
      }
   }

   ~MemoryEigen() {}

   // From AutoML-Zero https://doi.org/10.48550/arXiv.2003.03384
   // When modifying a real-valued constant, we multiply it by a
   // uniform random number in [0.5, 2.0] and flip its sign with
   // 10% probability
   inline void MutateConstants(std::mt19937 &rng) {
      auto dis1 = std::uniform_real_distribution<double>(0.5, 2.0);
      auto dis2 = std::uniform_real_distribution<double>(0.0, 1.0);
      for (size_t i = 0; i < const_memory_.size(); i++) {
         for (auto &x : const_memory_[i].reshaped()) {
            x *= dis1(rng);
            if (dis2(rng) <= 0.1) {
               x *= -1;
            }
         }
      }
   }

   inline void ClearConst() {
      for (auto &m : const_memory_) m.setZero();
   }

   inline void ClearWorking() {
      for (auto &m : working_memory_) m.setZero();
   }

   inline void ClearReadTime() {
      std::fill(read_time_.begin(), read_time_.end(), 0);
   }

   inline void ClearWriteTime() {
      std::fill(write_time_.begin(), write_time_.end(), 0);
   }

   inline void CopyConstToWorking() {
      for (size_t i = 0; i < const_memory_.size(); i++)
         working_memory_[i] = const_memory_[i];
   }

   inline void RandomizeConst() {
      for (auto &m : const_memory_) m.setRandom();
   }

   void ResizeMemory() {
      working_memory_.resize(n_memories_);
      const_memory_.resize(n_memories_);
      for (size_t i = 0; i < n_memories_; i++) {
         if (type_ == kScalarType_) {
            working_memory_[i].resize(1, 1);
            const_memory_[i].resize(1, 1);
         } else if (type_ == kVectorType_) {
            working_memory_[i].resize(memory_size_, 1);
            const_memory_[i].resize(memory_size_, 1);
         } else if (type_ == kMatrixType_) {
            working_memory_[i].resize(memory_size_, memory_size_);
            const_memory_[i].resize(memory_size_, memory_size_);
         }
      }
      read_time_.resize(n_memories_);
      write_time_.resize(n_memories_);
   }

   std::string ToString(long prog_id) {
      std::ostringstream oss;
      oss << "MemoryEigen:" << prog_id << ":" << type_ << ":" << n_memories_ << ":"
          << memory_size_;
      if (type_ == kScalarType_) {
         for (auto m : const_memory_) {
            oss << ":" << m(0, 0);
         }
      } else if (type_ == kVectorType_) {
         for (auto m : const_memory_) {
            for (size_t r = 0; r < memory_size_; r++) {
               oss << ":" << m(r, 0);
            }
         }
      } else {
         for (auto m : const_memory_) {
            for (size_t r = 0; r < memory_size_; r++) {
               for (size_t c = 0; c < memory_size_; c++) {
                  oss << ":" << m(r, c);
               }
            }
         }
      }
      oss << std::endl;
      return oss.str();
   }
};

// struct MemoryEigenIdComp {
//    bool operator()(MemoryEigen *m1, MemoryEigen *m2) const {
//       return m1->id_ < m2->id_;
//    }
// };

#endif
