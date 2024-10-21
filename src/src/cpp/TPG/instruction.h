#ifndef instruction_h
#define instruction_h

#include <state.h>
#include <bitset>
#include <iomanip>
#include <random>
#include <vector>

#include "memoryEigen.h"

class instruction {
 public:
  // Operations
  static const int SCALAR_SUM_OP_ = 0;
  static const int SCALAR_DIFF_OP_ = 1;
  static const int SCALAR_PRODUCT_OP_ = 2;
  static const int SCALAR_DIVISION_OP_ = 3;
  static const int SCALAR_ABS_OP_ = 4;
  static const int SCALAR_RECIPROCAL_OP_ = 5;
  static const int SCALAR_SIN_OP_ = 6;
  static const int SCALAR_COS_OP_ = 7;
  static const int SCALAR_TAN_OP_ = 8;
  static const int SCALAR_ARCSIN_OP_ = 9;
  static const int SCALAR_ARCCOS_OP_ = 10;
  static const int SCALAR_ARCTAN_OP_ = 11;
  static const int SCALAR_EXP_OP_ = 12;
  static const int SCALAR_LOG_OP_ = 13;
  static const int SCALAR_HEAVYSIDE_OP_ = 14;
  static const int VECTOR_HEAVYSIDE_OP_ = 15;
  static const int MATRIX_HEAVYSIDE_OP_ = 16;
  static const int SCALAR_VECTOR_PRODUCT_OP_ = 17;
  static const int SCALAR_BROADCAST_OP_ = 18;
  static const int VECTOR_RECIPROCAL_OP_ = 19;
  static const int VECTOR_NORM_OP_ = 20;
  static const int VECTOR_ABS_OP_ = 21;
  static const int VECTOR_SUM_OP_ = 22;
  static const int VECTOR_DIFF_OP_ = 23;
  static const int VECTOR_PRODUCT_OP_ = 24;
  static const int VECTOR_DIVISION_OP_ = 25;
  static const int VECTOR_INNER_PRODUCT_OP_ = 26;
  static const int VECTOR_OUTER_PRODUCT_OP_ = 27;
  static const int SCALAR_MATRIX_PRODUCT_OP_ = 28;
  static const int MATRIX_RECIPROCAL_OP_ = 29;
  static const int MATRIX_VECTOR_PRODUCT_OP_ = 30;
  static const int VECTOR_COLUMN_BROADCAST_OP_ = 31;
  static const int VECTOR_ROW_BROADCAST_OP_ = 32;
  static const int MATRIX_NORM_OP_ = 33;
  static const int MATRIX_COLUMN_NORM_OP_ = 34;
  static const int MATRIX_ROW_NORM_OP_ = 35;
  static const int MATRIX_TRANSPOSE_OP_ = 36;
  static const int MATRIX_ABS_OP_ = 37;
  static const int MATRIX_SUM_OP_ = 38;
  static const int MATRIX_DIFF_OP_ = 39;
  static const int MATRIX_PRODUCT_OP_ = 40;
  static const int MATRIX_DIVISION_OP_ = 41;
  static const int MATRIX_MATRIX_PRODUCT_OP_ = 42;
  static const int SCALAR_MIN_OP_ = 43;
  static const int VECTOR_MIN_OP_ = 44;
  static const int MATRIX_MIN_OP_ = 45;
  static const int SCALAR_MAX_OP_ = 46;
  static const int VECTOR_MAX_OP_ = 47;
  static const int MATRIX_MAX_OP_ = 48;
  static const int VECTOR_MEAN_OP_ = 49;
  static const int MATRIX_MEAN_OP_ = 50;
  static const int MATRIX_ROW_MEAN_OP_ = 51;
  static const int MATRIX_ROW_ST_DEV_OP_ = 52;
  static const int VECTOR_ST_DEV_OP_ = 53;
  static const int MATRIX_ST_DEV_OP_ = 54;
  static const int SCALAR_CONST_SET_OP_ = 55;
  static const int VECTOR_CONST_SET_OP_ = 56;
  static const int MATRIX_CONST_SET_OP_ = 57;
  static const int SCALAR_UNIFORM_SET_OP_ = 58;
  static const int VECTOR_UNIFORM_SET_OP_ = 59;
  static const int MATRIX_UNIFORM_SET_OP_ = 60;
  static const int SCALAR_GAUSSIAN_SET_OP_ = 61;
  static const int VECTOR_GAUSSIAN_SET_OP_ = 62;
  static const int MATRIX_GAUSSIAN_SET_OP_ = 63;
  static const int SCALAR_CONDITIONAL_OP_ = 64;
  static const int SCALAR_POW_OP_ = 65;
  static const int SCALAR_SQR_OP_ = 66;
  static const int SCALAR_CUBE_OP_ = 67;
  static const int SCALAR_TANH_OP_ = 68;
  static const int SCALAR_SQRT_OP_ = 69;
  static const int SCALAR_VECTOR_ASSIGN_OP_ = 70;
  static const int SCALAR_MATRIX_ASSIGN_OP_ = 71;
  static const int OBS_BUFF_SLICE_OP_ = 73;

  static const int NUM_OP = 74;

  static const vector<double> constants_;
  mt19937 rng_;

  // Mutable instruction parameters

  // Whether in1 is a memory or input reference
  // 0: memory ref
  // 1: observation ref
  int in1Src_ = 0;

  // Whether in2 is a memory or input reference
  // 0: memory ref
  // 1: observation ref
  int in2Src_ = 0;

  // Which memory index does this instruction write to
  int outIdx_ = 0;
  int outIdxE_ = 0;

  // Which operation does this instrcution execute
  int op_ = 0;

  // For memories, these index parameters specify which memory to use.
  // For observations, they specify which timestep in the buffer to use,
  // where 0 is the current observation, 1 is the previous observation, etc.
  // Their range is [0, memIndices_ - 1]
  int in0Idx_ = 0;
  int in1Idx_ = 0;
  int in0IdxE_ = 0;
  int in1IdxE_ = 0;

  // These parameters are used as indices to vector or matrix memory.
  // Their range is [0, memory_size_ - 1]
  int in2Idx_ = 0;
  int in3Idx_ = 0;
  int in2IdxE_ = 0;
  int in3IdxE_ = 0;

  /****************************************************************************/
  // The number of private memories of each type (scalar, vector, matrix)
  int memIndices_ = 0;

  // Scalor operation values are stored in these variables prior to execution.
  double scalar_out_ = 0;
  double scalar_in1_ = 0;
  double scalar_in2_ = 0;

  // Pointers to i/o for this instruction
  memoryEigen* out_;
  memoryEigen* in1_;
  memoryEigen* in2_;

  // Dimensionality of vector and martrix memory
  // Vector memories will be shape (memory_size_,1)
  // Matrices will be shape (memory_size_,memory_size_)
  int memory_size_ = 0;

  // Maps operations to memory types for {out, in1, in2}
  // Each operation requires unique i/o memory types. For example:
  //  op_mem_types_[SCALAR_SUM_OP_] = {SCALAR_TYPE, SCALAR_TYPE, SCALAR_TYPE};
  //  op_mem_types_[SCALAR_COS_OP_] = {SCALAR_TYPE, SCALAR_TYPE, NA_TYPE};
  // NA_TYPE is a place holder for ops which only require one input (in1).
  static vector<vector<size_t> > op_mem_types_;

  string checkpoint();

  // Constructor
  instruction(std::unordered_map<string, std::any>&, mt19937&);

  // Copy Constructor
  instruction(instruction&);

  // op_list_ maps each operation to a function pointer for its execution
  typedef void (instruction::*operation)(bool);
  static vector<operation> op_list_;

  // Execute this instruction
  inline void exec(bool dbg) {
    (this->*op_list_[op_])(dbg);

    // TODO(skelly): set to 1.0 instead of 0.0?
    // Change infinite values to 0.0 in output memory
    // This "protects" output memory by filtering nan value.
    out_->working_memory_[outIdxE_].array() =
        out_->working_memory_[outIdxE_].array().unaryExpr(
            [](double v) { return std::isfinite(v) ? v : 0.0; });
  }

  inline int GetInIdx(int i) const {
    if (i == 0)
      return in0Idx_;
    else if (i == 1)
      return in1Idx_;
    else if (i == 2)
      return in2Idx_;
    else
      return in3Idx_;
  }

  inline int GetInIdxE(int i) const {
    if (i == 0)
      return in0IdxE_;
    else if (i == 1)
      return in1IdxE_;
    else if (i == 2)
      return in2IdxE_;
    else
      return in3IdxE_;
  }

  inline void SetInIdx(int i, int idx) {
    if (i == 0)
      in0Idx_ = idx;
    else if (i == 1)
      in1Idx_ = idx;
    else if (i == 2)
      in2Idx_ = idx;
    else
      in3Idx_ = idx;
  }

  inline void SetInIdxE(int i, int idx) {
    if (i == 0)
      in0IdxE_ = idx;
    else if (i == 1)
      in1IdxE_ = idx;
    else if (i == 2)
      in2IdxE_ = idx;
    else
      in3IdxE_ = idx;
  }

  inline memoryEigen* GetInMem(int i) const { return i == 0 ? in1_ : in2_; }
  inline void SetInMem(int i, memoryEigen* m) { (i == 0 ? in1_ : in2_) = m; }
  inline size_t GetInType(int i) const { return op_mem_types_[op_][i + 1]; }
  inline bool IsObs(int i) const {
    return (GetInType(i) != memoryEigen::NA_TYPE) &&
           (i == 0 ? in1Src_ == 1 : in2Src_ == 1);
  }
  inline bool IsMemoryRef(int i) const {
    return !IsObs(i) && GetInType(i) != memoryEigen::NA_TYPE;
  }
  void Mutate(bool randomize, vector<bool>& legal_ops,
              int observation_buff_size, mt19937& rng);
  inline size_t GetOutType() const { return op_mem_types_[op_][0]; }
  static void SetupOps();

  // For operations in which a scalar refers to an observation ref, we take its
  // value from an index into the vector input buffer.
  // The scalar input buffer is never used.
  void SetupScalarIn(int in, state* obs) {
    double* scalar = in == 0 ? &scalar_in1_ : &scalar_in2_;
    int* index = in == 0 ? &in0Idx_ : &in1Idx_;
    if (IsObs(in)) {
      // TODO (skelly): should this also be range limited to memory_size?
      *scalar = obs->stateValueAtIndex(*index % obs->dim_);
      // TODO(skelly): debugging output
      // cerr << "scalar_in idx "  << " val " << *scalar << endl;
    } else {
      memoryEigen* input_memory = in == 0 ? in1_ : in2_;
      *scalar =
          input_memory->working_memory_[*index % GetInMem(in)->memoryIndices_](
              0, 0);
    }
  }

  void BoundMemoryIndices(int observation_buff_size);

  void MutateInt(int& i, int min, int max, mt19937& rng) {
    if (min != max) {
      auto dis = std::uniform_int_distribution<>(min, max);
      auto prev = i;
      do {
        i = dis(rng);
      } while (i == prev);
    }
  }

  /* Operation implementations ************************************************/

  inline void ExecuteScalarSumOp(bool dbg) {
    out_->working_memory_[outIdxE_](0, 0) = scalar_in1_ + scalar_in2_;

    if (dbg) {
      cerr << std::setprecision(std::numeric_limits<double>::digits10 + 1)
           << std::fixed << "s" << outIdxE_ << " = s" << in0IdxE_
           << (IsObs(0) ? "i" : "") << " + " << "s" << in1IdxE_
           << (IsObs(1) ? "i" : "") << " | " << std::fixed << scalar_in1_
           << " + " << scalar_in2_ << " = "
           << out_->working_memory_[outIdxE_](0, 0) << endl;
    }
  }

  inline void ExecuteScalarDiffOp(bool dbg) {
    out_->working_memory_[outIdxE_](0, 0) = scalar_in1_ - scalar_in2_;

    if (dbg) {
      cerr << std::setprecision(std::numeric_limits<double>::digits10 + 1)
           << std::fixed << "s" << outIdxE_ << " = s" << in0IdxE_
           << (IsObs(0) ? "i" : "") << " - " << "s" << in1IdxE_
           << (IsObs(1) ? "i" : "") << " | " << std::fixed << scalar_in1_
           << " - " << scalar_in2_ << " = "
           << out_->working_memory_[outIdxE_](0, 0) << endl;
    }
  }

  inline void ExecuteScalarProductOp(bool dbg) {
    out_->working_memory_[outIdxE_](0, 0) = scalar_in1_ * scalar_in2_;

    if (dbg) {
      cerr << std::setprecision(std::numeric_limits<double>::digits10 + 1)
           << std::fixed << "s" << outIdxE_ << " = s" << in0IdxE_
           << (IsObs(0) ? "i" : "") << " * " << "s" << in1IdxE_
           << (IsObs(1) ? "i" : "") << " | " << std::fixed << scalar_in1_
           << " * " << scalar_in2_ << " = "
           << out_->working_memory_[outIdxE_](0, 0) << endl;
    }
  }

  inline void ExecuteScalarDivisionOp(bool dbg) {
    // Protected division
    if (isEqual(scalar_in2_, 0.0)) {
      out_->working_memory_[outIdxE_](0, 0) = 0;  // TODO(skelly): 1.0 instead?
    } else {
      out_->working_memory_[outIdxE_](0, 0) = scalar_in1_ / scalar_in2_;
    }

    if (dbg) {
      cerr << std::setprecision(std::numeric_limits<double>::digits10 + 1)
           << std::fixed << "s" << outIdxE_ << " = s" << in0IdxE_ << " / " << "s"
           << in1IdxE_ << " | " << std::fixed << scalar_in1_ << " / "
           << scalar_in2_ << " = " << out_->working_memory_[outIdxE_](0, 0)
           << endl;
    }
  }

  inline void ExecuteScalarReciprocalOp(bool dbg) {
    out_->working_memory_[outIdxE_](0, 0) = 1.0 / scalar_in1_;

    if (dbg) {
    }
  }

  inline void ExecuteScalarAbsOp(bool dbg) {
    out_->working_memory_[outIdxE_](0, 0) = std::abs(scalar_in1_);

    if (dbg) {
      cerr << "s" << outIdxE_ << " = abs(s" << in0IdxE_ << ") | " << "abs("
           << scalar_in1_ << ") = " << out_->working_memory_[outIdxE_](0, 0)
           << endl;
    }
  }

  inline void ExecuteScalarSinOp(bool dbg) {
    out_->working_memory_[outIdxE_](0, 0) = std::sin(scalar_in1_);

    if (dbg) {
      cerr << "s" << outIdxE_ << " = sin(s" << in0IdxE_ << ") | " << "sin("
           << scalar_in1_ << ") = " << out_->working_memory_[outIdxE_](0, 0)
           << endl;
    }
  }

  inline void ExecuteScalarCosOp(bool dbg) {
    out_->working_memory_[outIdxE_](0, 0) = std::cos(scalar_in1_);

    if (dbg) {
      cerr << "s" << outIdxE_ << " = cos(s" << in0IdxE_ << (IsObs(0) ? "i" : "")
           << ") | " << "cos(" << scalar_in1_
           << ") = " << out_->working_memory_[outIdxE_](0, 0) << endl;
    }
  }

  inline void ExecuteScalarTanOp(bool dbg) {
    out_->working_memory_[outIdxE_](0, 0) = std::tan(scalar_in1_);

    if (dbg) {
      cerr << "s" << outIdxE_ << " = tan(s" << in0IdxE_ << ") | " << "tan("
           << scalar_in1_ << ") = " << out_->working_memory_[outIdxE_](0, 0)
           << endl;
    }
  }

  inline void ExecuteScalarExpOp(bool dbg) {
    out_->working_memory_[outIdxE_](0, 0) = std::exp(scalar_in1_);

    if (dbg) {
      cerr << "s" << outIdxE_ << " = exp(s" << in0IdxE_ << ") | " << "exp("
           << scalar_in1_ << ") = " << out_->working_memory_[outIdxE_](0, 0)
           << endl;
    }
  }

  inline void ExecuteScalarLogOp(bool dbg) {
    out_->working_memory_[outIdxE_](0, 0) = std::log(scalar_in1_);

    if (dbg) {
      cerr << "s" << outIdxE_ << " = log(s" << in0IdxE_ << ") | " << "log("
           << scalar_in1_ << ") = " << out_->working_memory_[outIdxE_](0, 0)
           << endl;
    }
  }

  inline void ExecuteScalarArcSinOp(bool dbg) {
    out_->working_memory_[outIdxE_](0, 0) = std::asin(scalar_in1_);

    if (dbg) {
      cerr << "s" << outIdxE_ << " = asin(s" << in0IdxE_ << ") | " << "asin("
           << scalar_in1_ << ") = " << out_->working_memory_[outIdxE_](0, 0)
           << endl;
    }
  }

  inline void ExecuteScalarArcCosOp(bool dbg) {
    out_->working_memory_[outIdxE_](0, 0) = std::acos(scalar_in1_);

    if (dbg) {
      cerr << "s" << outIdxE_ << " = acos(s" << in0IdxE_ << ") | " << "acos("
           << scalar_in1_ << ") = " << out_->working_memory_[outIdxE_](0, 0)
           << endl;
    }
  }

  inline void ExecuteScalarArcTanOp(bool dbg) {
    out_->working_memory_[outIdxE_](0, 0) = std::atan(scalar_in1_);
    if (dbg) {
      cerr << "s" << outIdxE_ << " = atan(s" << in0IdxE_ << ") | " << "atan("
           << scalar_in1_ << ") = " << out_->working_memory_[outIdxE_](0, 0)
           << endl;
    }
  }

  inline void ExecuteScalarHeavisideOp(bool dbg) {
    out_->working_memory_[outIdxE_](0, 0) = scalar_in1_ >= 0.0 ? 1.0 : 0.0;

    if (dbg) {
    }
  }

  inline void ExecuteVectorHeavisideOp(bool dbg) {
    const double* in = in1_->working_memory_[in0IdxE_].data();
    const double* in_end = in + memory_size_;
    double* out = out_->working_memory_[outIdxE_].data();
    while (in != in_end) {
      *out = *in > 0.0 ? 1.0 : 0.0;
      ++out;
      ++in;
    }
    if (dbg) {
    }
  }

  inline void ExecuteMatrixHeavisideOp(bool dbg) {
    const double* ind = in1_->working_memory_[in0IdxE_].data();
    const double* ind_end =
        ind + memory_size_ * memory_size_;  // all matices will be same size
    double* outd = out_->working_memory_[outIdxE_].data();
    while (ind != ind_end) {
      *outd = *ind > 0.0 ? 1.0 : 0.0;
      ++outd;
      ++ind;
    }
    if (dbg) {
    }
  }

  inline void ExecuteScalarVectorProductOp(bool dbg) {
    out_->working_memory_[outIdxE_] =
        in1_->working_memory_[in0IdxE_](0, 0) * in2_->working_memory_[in1IdxE_];
    if (dbg) {
    }
  }

  inline void ExecuteScalarBroadcastOp(bool dbg) {
    out_->working_memory_[outIdxE_] =
        in1_->working_memory_[in0IdxE_](0, 0) * MatrixXd::Ones(memory_size_, 1);
    if (dbg) {
    }
  }

  inline void ExecuteVectorReciprocalOp(bool dbg) {
    out_->working_memory_[outIdxE_](0, 0) =
        1.0 / in1_->working_memory_[in0IdxE_](0, 0);
    if (dbg) {
    }
  }

  inline void ExecuteVectorNormOp(bool dbg) {
    out_->working_memory_[outIdxE_](0, 0) =
        in1_->working_memory_[in0IdxE_].norm();
    if (dbg) {
    }
  }

  inline void ExecuteVectorAbsOp(bool dbg) {
    out_->working_memory_[outIdxE_] =
        (in1_->working_memory_[in0IdxE_].array().abs()).matrix();
    if (dbg) {
    }
  }

  inline void ExecuteVectorSumOp(bool dbg) {
    out_->working_memory_[outIdxE_] =
        in1_->working_memory_[in0IdxE_] + in2_->working_memory_[in1IdxE_];
    if (dbg) {
    }
  }

  inline void ExecuteVectorDiffOp(bool dbg) {
    out_->working_memory_[outIdxE_] =
        in1_->working_memory_[in0IdxE_] - in2_->working_memory_[in1IdxE_];
    if (dbg) {
    }
  }

  inline void ExecuteVectorProductOp(bool dbg) {
    // cerr << "outIdxE_ " << outIdxE_ << " in0IdxE_ " << in0IdxE_ << " in1IdxE_ " << in1IdxE_;
    // cerr << " " <<  out_->working_memory_[outIdxE_].rows() << "," << out_->working_memory_[outIdxE_].cols();
    // cerr << " " <<  in1_->working_memory_[in0IdxE_].rows() << "," << in1_->working_memory_[in0IdxE_].cols();
    // cerr << " " <<  in2_->working_memory_[in1IdxE_].rows() << "," << in2_->working_memory_[in1IdxE_].cols() << endl;

    out_->working_memory_[outIdxE_] = in1_->working_memory_[in0IdxE_].array() *
                                     in2_->working_memory_[in1IdxE_].array();
    if (dbg) {
    }
  }

  inline void ExecuteVectorDivisionOp(bool dbg) {
    out_->working_memory_[outIdxE_] = in1_->working_memory_[in0IdxE_].array() /
                                     in2_->working_memory_[in1IdxE_].array();
    if (dbg) {
    }
  }

  inline void ExecuteVectorInnerProductOp(bool dbg) {
    out_->working_memory_[outIdxE_](0, 0) =
        in1_->working_memory_[in0IdxE_].col(0).dot(
            in2_->working_memory_[in1IdxE_].col(0));
    if (dbg) {
    }
  }

  inline void ExecuteVectorOuterProductOp(bool dbg) {
    out_->working_memory_[outIdxE_] = in1_->working_memory_[in0IdxE_] *
                                     in2_->working_memory_[in1IdxE_].transpose();
    if (dbg) {
    }
  }

  inline void ExecuteScalarMatrixProductOp(bool dbg) {
    out_->working_memory_[outIdxE_] =
        in1_->working_memory_[in0IdxE_](0, 0) * in2_->working_memory_[in1IdxE_];
    if (dbg) {
    }
  }

  inline void ExecuteMatrixReciprocalOp(bool dbg) {
    out_->working_memory_[outIdxE_] =
        (1.0 / in1_->working_memory_[in0IdxE_].array()).matrix();
    if (dbg) {
    }
  }

  inline void ExecuteMatrixVectorProductOp(bool dbg) {
    // cerr <<"dbg " << out_->working_memory_[outIdxE_].rows() << " " << in1_->working_memory_[in0IdxE_].rows() << " " << in2_->working_memory_[in1IdxE_].rows();
    // cerr << " types " << GetInType(0) << " " << GetInType(1);
    // cerr << " src " << in1Src_ << " " << in2Src_;
    // cerr << " memory_size " << memory_size_ << endl;
    out_->working_memory_[outIdxE_] =
        in1_->working_memory_[in0IdxE_] * in2_->working_memory_[in1IdxE_];
    if (dbg) {
    }
  }

  inline void ExecuteVectorColumnBroadcastOp(bool dbg) {
    out_->working_memory_[outIdxE_] = in1_->working_memory_[in0IdxE_].replicate(
        1, in1_->working_memory_[in0IdxE_].rows());
    if (dbg) {
    }
  }

  inline void ExecuteVectorRowBroadcastOp(bool dbg) {
    out_->working_memory_[outIdxE_] =
        in1_->working_memory_[in0IdxE_]
            .replicate(1, in1_->working_memory_[in0IdxE_].rows())
            .transpose();
    if (dbg) {
    }
  }

  inline void ExecuteMatrixNormOp(bool dbg) {
    out_->working_memory_[outIdxE_](0, 0) =
        in1_->working_memory_[in0IdxE_].norm();
    if (dbg) {
    }
  }

  inline void ExecuteMatrixColumnNormOp(bool dbg) {
    out_->working_memory_[outIdxE_] =
        in1_->working_memory_[in0IdxE_]
            .colwise()
            .norm()
            .transpose();  // automl-zero doesn't transpose?
    if (dbg) {
    }
  }

  inline void ExecuteMatrixRowNormOp(bool dbg) {
    out_->working_memory_[outIdxE_] =
        in1_->working_memory_[in0IdxE_].rowwise().norm();
    if (dbg) {
    }
  }

  inline void ExecuteMatrixTransposeOp(bool dbg) {
    if (outIdxE_ == in0IdxE_)
      out_->working_memory_[outIdxE_].transposeInPlace();
    else
      out_->working_memory_[outIdxE_] =
          in1_->working_memory_[in0IdxE_].transpose();
    if (dbg) {
    }
  }

  inline void ExecuteMatrixAbsOp(bool dbg) {
    out_->working_memory_[outIdxE_] =
        in1_->working_memory_[in0IdxE_].array().abs().matrix();
    if (dbg) {
    }
  }

  inline void ExecuteMatrixSumOp(bool dbg) {
    out_->working_memory_[outIdxE_] =
        in1_->working_memory_[in0IdxE_] + in2_->working_memory_[in1IdxE_];
    if (dbg) {
    }
  }

  inline void ExecuteMatrixDiffOp(bool dbg) {
    out_->working_memory_[outIdxE_] =
        in1_->working_memory_[in0IdxE_] - in2_->working_memory_[in1IdxE_];
    if (dbg) {
    }
  }

  inline void ExecuteMatrixProductOp(bool dbg) {
    out_->working_memory_[outIdxE_] = (in1_->working_memory_[in0IdxE_].array() *
                                      in2_->working_memory_[in1IdxE_].array())
                                         .matrix();
    if (dbg) {
    }
  }

  inline void ExecuteMatrixDivisionOp(bool dbg) {
    out_->working_memory_[outIdxE_] = (in1_->working_memory_[in0IdxE_].array() /
                                      in2_->working_memory_[in1IdxE_].array())
                                         .matrix();
    if (dbg) {
    }
  }

  inline void ExecuteMatrixMatrixProductOp(bool dbg) {
    out_->working_memory_[outIdxE_] =
        in1_->working_memory_[in0IdxE_] * in2_->working_memory_[in1IdxE_];
    if (dbg) {
    }
  }

  inline void ExecuteScalarMinOp(bool dbg) {
    out_->working_memory_[outIdxE_](0, 0) = min(scalar_in1_, scalar_in2_);

    if (dbg) {
    }
  }

  inline void ExecuteVectorMinOp(bool dbg) {
    out_->working_memory_[outIdxE_] = in1_->working_memory_[in0IdxE_].array().min(
        in2_->working_memory_[in1IdxE_].array());
    if (dbg) {
    }
  }

  inline void ExecuteMatrixMinOp(bool dbg) {
    const double* in1d = in1_->working_memory_[in0IdxE_].data();
    const double* in2d = in2_->working_memory_[in1IdxE_].data();
    const double* in1_end = in1d + in2_->working_memory_[in1IdxE_].rows() *
                                       in2_->working_memory_[in1IdxE_].cols();
    double* outd = out_->working_memory_[outIdxE_].data();
    while (in1d != in1_end) {
      const double in1v = *in1d;
      const double in2v = *in2d;
      *outd = in1v < in2v ? in1v : in2v;
      ++outd;
      ++in1d;
      ++in2d;
    }
    if (dbg) {
    }
  }

  inline void ExecuteScalarMaxOp(bool dbg) {
    out_->working_memory_[outIdxE_](0, 0) = max(scalar_in1_, scalar_in2_);

    if (dbg) {
    }
  }

  inline void ExecuteVectorMaxOp(bool dbg) {
    out_->working_memory_[outIdxE_] = in1_->working_memory_[in0IdxE_].array().max(
        in2_->working_memory_[in1IdxE_].array());
    if (dbg) {
    }
  }

  inline void ExecuteMatrixMaxOp(bool dbg) {
    const double* in1d = in1_->working_memory_[in0IdxE_].data();
    const double* in2d = in2_->working_memory_[in1IdxE_].data();
    const double* in1_end = in1d + in2_->working_memory_[in1IdxE_].rows() *
                                       in2_->working_memory_[in1IdxE_].cols();
    double* outd = out_->working_memory_[outIdxE_].data();
    while (in1d != in1_end) {
      const double in1v = *in1d;
      const double in2v = *in2d;
      *outd = in1v > in2v ? in1v : in2v;
      ++outd;
      ++in1d;
      ++in2d;
    }
    if (dbg) {
    }
  }

  inline void ExecuteVectorMeanOp(bool dbg) {
    out_->working_memory_[outIdxE_](0, 0) =
        in1_->working_memory_[in0IdxE_].mean();
    if (dbg) {
    }
  }

  inline void ExecuteMatrixMeanOp(bool dbg) {
    out_->working_memory_[outIdxE_](0, 0) =
        in1_->working_memory_[in0IdxE_].mean();
    if (dbg) {
    }
  }

  inline void ExecuteMatrixRowMeanOp(bool dbg) {
    out_->working_memory_[outIdxE_] =
        in1_->working_memory_[in0IdxE_].rowwise().mean();
    if (dbg) {
    }
  }

  inline void ExecuteMatrixRowStDevOp(bool dbg) {
    for (int row = 0; row < out_->working_memory_[outIdxE_].rows(); ++row) {
      const MatrixXd values = in1_->working_memory_[in0IdxE_].row(row);
      const double mean = values.mean();
      const double stdev =
          sqrt((values.array() * values.array()).sum() /
                   static_cast<double>(in1_->working_memory_[in0IdxE_].rows()) -
               mean * mean);
      out_->working_memory_[outIdxE_](row, 0) = stdev;
    }
    if (dbg) {
    }
  }

  inline void ExecuteVectorStDevOp(bool dbg) {
    const double mean = in1_->working_memory_[in0IdxE_].mean();
    out_->working_memory_[outIdxE_](0, 0) =
        sqrt(in1_->working_memory_[in0IdxE_].col(0).dot(
                 in1_->working_memory_[in0IdxE_].col(0)) /
                 in1_->working_memory_[in0IdxE_].rows() -
             mean * mean);
    if (dbg) {
    }
  }

  inline void ExecuteMatrixStDevOp(bool dbg) {
    const MatrixXd values = in1_->working_memory_[in0IdxE_];
    const double mean = values.mean();
    out_->working_memory_[outIdxE_](0, 0) =
        sqrt((values.array() * values.array()).sum() /
                 static_cast<double>(out_->working_memory_[outIdxE_].rows() *
                                     out_->working_memory_[outIdxE_].cols()) -
             mean * mean);
    if (dbg) {
    }
  }

  inline void ExecuteScalarConstSetOp(bool dbg) {
    out_->working_memory_[outIdxE_] = in1_->const_memory_[in0IdxE_];
    if (dbg) {
    }
  }

  inline void ExecuteVectorConstSetOp(bool dbg) {
    out_->working_memory_[outIdxE_] = in1_->const_memory_[in0IdxE_];

    if (dbg) {
    }
  }

  inline void ExecuteMatrixConstSetOp(bool dbg) {
    out_->working_memory_[outIdxE_] = in1_->const_memory_[in0IdxE_];

    if (dbg) {
    }
  }

  inline void ExecuteScalarUniformSetOp(bool dbg) {
    uniform_real_distribution<double> dis(in1_->working_memory_[in0IdxE_](0, 0),
                                          in2_->working_memory_[in1IdxE_](0, 0));
    out_->working_memory_[outIdxE_](0, 0) = dis(rng_);
    if (dbg) {
    }
  }

  inline void ExecuteVectorUniformSetOp(bool dbg) {
    uniform_real_distribution<double> dis(in1_->working_memory_[in0IdxE_](0, 0),
                                          in2_->working_memory_[in1IdxE_](0, 0));
    for (int i = 0; i < out_->working_memory_[outIdxE_].rows(); i++)
      out_->working_memory_[outIdxE_](i, 0) = dis(rng_);
    if (dbg) {
    }
  }

  inline void ExecuteMatrixUniformSetOp(bool dbg) {
    uniform_real_distribution<double> dis(in1_->working_memory_[in0IdxE_](0, 0),
                                          in2_->working_memory_[in1IdxE_](0, 0));
    for (int row = 0; row < out_->working_memory_[outIdxE_].rows(); row++)
      for (int col = 0; col < out_->working_memory_[outIdxE_].cols(); col++)
        out_->working_memory_[outIdxE_](row, col) = dis(rng_);
    if (dbg) {
    }
  }

  inline void ExecuteScalarGaussianSetOp(bool dbg) {
    normal_distribution<double> dis(in1_->working_memory_[in0IdxE_](0, 0),
                                    in2_->working_memory_[in1IdxE_](0, 0));
    out_->working_memory_[outIdxE_](0, 0) = dis(rng_);
    if (dbg) {
    }
  }

  inline void ExecuteVectorGaussianSetOp(bool dbg) {
    normal_distribution<double> dis(in1_->working_memory_[in0IdxE_](0, 0),
                                    in2_->working_memory_[in1IdxE_](0, 0));
    for (int i = 0; i < out_->working_memory_[outIdxE_].rows(); i++)
      out_->working_memory_[outIdxE_](i, 0) = dis(rng_);
    if (dbg) {
    }
  }

  inline void ExecuteMatrixGaussianSetOp(bool dbg) {
    normal_distribution<double> dis(in1_->working_memory_[in0IdxE_](0, 0),
                                    in2_->working_memory_[in1IdxE_](0, 0));
    for (int row = 0; row < out_->working_memory_[outIdxE_].rows(); row++)
      for (int col = 0; col < out_->working_memory_[outIdxE_].cols(); col++)
        out_->working_memory_[outIdxE_](row, col) = dis(rng_);
    if (dbg) {
    }
  }

  inline void ExecuteScalarConditionalOp(bool dbg) {
    if (scalar_in1_ < scalar_in2_)
      out_->working_memory_[outIdxE_](0, 0) =
          -(out_->working_memory_[outIdxE_](0, 0));

    if (dbg) {
      cerr << " IF s" << in0IdxE_ << " < s" << in1IdxE_ << " THEN s" << outIdxE_
           << " = -s" << outIdxE_ << " | " << " in1 " << scalar_in1_ << " in2 "
           << scalar_in2_ << " : " << out_->working_memory_[outIdxE_](0, 0)
           << endl;
    }
  }

  inline void ExecuteScalarPowOp(bool dbg) {
    out_->working_memory_[outIdxE_](0, 0) = std::pow(scalar_in1_, scalar_in2_);
    if (dbg) {
      cerr << "s" << outIdxE_ << " = pow(s" << in0IdxE_ << ", s" << in1IdxE_
           << ") | " << "pow(" << scalar_in1_ << ", " << scalar_in2_ << " = "
           << out_->working_memory_[outIdxE_](0, 0) << endl;
    }
  }

  inline void ExecuteScalarSqrOp(bool dbg) {
    out_->working_memory_[outIdxE_](0, 0) = std::pow(scalar_in1_, 2);

    if (dbg) {
    }
  }

  inline void ExecuteScalarCubeOp(bool dbg) {
    out_->working_memory_[outIdxE_](0, 0) = std::pow(scalar_in1_, 2);

    if (dbg) {
    }
  }

  inline void ExecuteScalarTanhOp(bool dbg) {
    out_->working_memory_[outIdxE_](0, 0) = std::tanh(scalar_in1_);

    if (dbg) {
    }
  }

  inline void ExecuteScalarSqrtOp(bool dbg) {
    out_->working_memory_[outIdxE_](0, 0) = std::sqrt(scalar_in1_);

    if (dbg) {
    }
  }

  inline void ExecuteScalarVectorAssignOp(bool dbg) {
    out_->working_memory_[outIdxE_](0, 0) =
        in1_->working_memory_[in0IdxE_](in2IdxE_, 0);
    if (dbg) {
    }
  }

  inline void ExecuteScalarMatrixAssignOp(bool dbg) {
    out_->working_memory_[outIdxE_](0, 0) =
        in1_->working_memory_[in0IdxE_](in2IdxE_, in3IdxE_);
    if (dbg) {
    }
  }

  // TODO(skelly): warning: this op assumes obs_buff == memory_size
  // so, can't evolve obs buff size
  inline void ExecuteObsBuffSliceOp(bool dbg) {
    for (size_t i = 0; i < in1_->working_memory_.size(); i++) {
      out_->working_memory_[outIdxE_](i, 0) =
          in1_->working_memory_[i](in2IdxE_, 0);
    }

    if (dbg) {
      // TO(skelly): improve debug format
      cerr << "from ";
      for (size_t i = 0; i < in1_->working_memory_.size(); i++) {
        cerr << " " << in1_->working_memory_[i](in2IdxE_, 0);
      }
      cerr << " To:";
      for (size_t i = 0; i < out_->working_memory_.size(); i++) {
        cerr << " " << out_->working_memory_[outIdxE_](i, 0);
      }
      cerr << endl;
    }
  }
};
#endif
