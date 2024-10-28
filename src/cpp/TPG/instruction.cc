#include "instruction.h"

vector<vector<size_t> > instruction::op_mem_types_(NUM_OP);
vector<instruction::operation> instruction::op_list_(NUM_OP);

string instruction::checkpoint() {
  ostringstream oss;
  oss << in1Src_ << "_";
  oss << in2Src_ << "_";
  oss << outIdx_ << "_";
  oss << op_ << "_";
  oss << in0Idx_ << "_";
  oss << in1Idx_ << "_";
  oss << in2Idx_ << "_";
  oss << in3Idx_ << "_";
  oss << memory_size_ << "_";
  return oss.str();
}

void instruction::SetupOps() {
  op_mem_types_[SCALAR_SUM_OP_] = {memoryEigen::SCALAR_TYPE,
                                   memoryEigen::SCALAR_TYPE,
                                   memoryEigen::SCALAR_TYPE};
  op_list_[SCALAR_SUM_OP_] = (&instruction::ExecuteScalarSumOp);

  op_mem_types_[SCALAR_DIFF_OP_] = {memoryEigen::SCALAR_TYPE,
                                    memoryEigen::SCALAR_TYPE,
                                    memoryEigen::SCALAR_TYPE};
  op_list_[SCALAR_DIFF_OP_] = (&instruction::ExecuteScalarDiffOp);

  op_mem_types_[SCALAR_PRODUCT_OP_] = {memoryEigen::SCALAR_TYPE,
                                       memoryEigen::SCALAR_TYPE,
                                       memoryEigen::SCALAR_TYPE};
  op_list_[SCALAR_PRODUCT_OP_] = (&instruction::ExecuteScalarProductOp);

  op_mem_types_[SCALAR_DIVISION_OP_] = {memoryEigen::SCALAR_TYPE,
                                        memoryEigen::SCALAR_TYPE,
                                        memoryEigen::SCALAR_TYPE};
  op_list_[SCALAR_DIVISION_OP_] = (&instruction::ExecuteScalarDivisionOp);

  op_mem_types_[SCALAR_ABS_OP_] = {
      memoryEigen::SCALAR_TYPE, memoryEigen::SCALAR_TYPE, memoryEigen::NA_TYPE};
  op_list_[SCALAR_ABS_OP_] = (&instruction::ExecuteScalarAbsOp);

  op_mem_types_[SCALAR_SIN_OP_] = {
      memoryEigen::SCALAR_TYPE, memoryEigen::SCALAR_TYPE, memoryEigen::NA_TYPE};
  op_list_[SCALAR_SIN_OP_] = (&instruction::ExecuteScalarSinOp);

  op_mem_types_[SCALAR_COS_OP_] = {
      memoryEigen::SCALAR_TYPE, memoryEigen::SCALAR_TYPE, memoryEigen::NA_TYPE};
  op_list_[SCALAR_COS_OP_] = (&instruction::ExecuteScalarCosOp);

  op_mem_types_[SCALAR_TAN_OP_] = {
      memoryEigen::SCALAR_TYPE, memoryEigen::SCALAR_TYPE, memoryEigen::NA_TYPE};
  op_list_[SCALAR_TAN_OP_] = (&instruction::ExecuteScalarTanOp);

  op_mem_types_[SCALAR_EXP_OP_] = {
      memoryEigen::SCALAR_TYPE, memoryEigen::SCALAR_TYPE, memoryEigen::NA_TYPE};
  op_list_[SCALAR_EXP_OP_] = (&instruction::ExecuteScalarExpOp);

  op_mem_types_[SCALAR_LOG_OP_] = {
      memoryEigen::SCALAR_TYPE, memoryEigen::SCALAR_TYPE, memoryEigen::NA_TYPE};
  op_list_[SCALAR_LOG_OP_] = (&instruction::ExecuteScalarLogOp);

  op_mem_types_[SCALAR_RECIPROCAL_OP_] = {
      memoryEigen::SCALAR_TYPE, memoryEigen::SCALAR_TYPE, memoryEigen::NA_TYPE};
  op_list_[SCALAR_RECIPROCAL_OP_] = (&instruction::ExecuteScalarReciprocalOp);

  op_mem_types_[SCALAR_SIN_OP_] = {
      memoryEigen::SCALAR_TYPE, memoryEigen::SCALAR_TYPE, memoryEigen::NA_TYPE};
  op_list_[SCALAR_SIN_OP_] = (&instruction::ExecuteScalarSinOp);

  op_mem_types_[SCALAR_COS_OP_] = {
      memoryEigen::SCALAR_TYPE, memoryEigen::SCALAR_TYPE, memoryEigen::NA_TYPE};
  op_list_[SCALAR_COS_OP_] = (&instruction::ExecuteScalarCosOp);

  op_mem_types_[SCALAR_ARCSIN_OP_] = {
      memoryEigen::SCALAR_TYPE, memoryEigen::SCALAR_TYPE, memoryEigen::NA_TYPE};
  op_list_[SCALAR_ARCSIN_OP_] = (&instruction::ExecuteScalarArcSinOp);

  op_mem_types_[SCALAR_ARCCOS_OP_] = {
      memoryEigen::SCALAR_TYPE, memoryEigen::SCALAR_TYPE, memoryEigen::NA_TYPE};
  op_list_[SCALAR_ARCCOS_OP_] = (&instruction::ExecuteScalarArcCosOp);

  op_mem_types_[SCALAR_ARCTAN_OP_] = {
      memoryEigen::SCALAR_TYPE, memoryEigen::SCALAR_TYPE, memoryEigen::NA_TYPE};
  op_list_[SCALAR_ARCTAN_OP_] = (&instruction::ExecuteScalarArcTanOp);

  op_mem_types_[SCALAR_HEAVYSIDE_OP_] = {
      memoryEigen::SCALAR_TYPE, memoryEigen::SCALAR_TYPE, memoryEigen::NA_TYPE};
  op_list_[SCALAR_HEAVYSIDE_OP_] = (&instruction::ExecuteScalarHeavisideOp);

  op_mem_types_[VECTOR_HEAVYSIDE_OP_] = {
      memoryEigen::VECTOR_TYPE, memoryEigen::VECTOR_TYPE, memoryEigen::NA_TYPE};
  op_list_[VECTOR_HEAVYSIDE_OP_] = (&instruction::ExecuteVectorHeavisideOp);

  op_mem_types_[MATRIX_HEAVYSIDE_OP_] = {
      memoryEigen::MATRIX_TYPE, memoryEigen::MATRIX_TYPE, memoryEigen::NA_TYPE};
  op_list_[MATRIX_HEAVYSIDE_OP_] = (&instruction::ExecuteMatrixHeavisideOp);

  op_mem_types_[SCALAR_VECTOR_PRODUCT_OP_] = {memoryEigen::VECTOR_TYPE,
                                              memoryEigen::SCALAR_TYPE,
                                              memoryEigen::VECTOR_TYPE};
  op_list_[SCALAR_VECTOR_PRODUCT_OP_] =
      (&instruction::ExecuteScalarVectorProductOp);

  op_mem_types_[SCALAR_BROADCAST_OP_] = {
      memoryEigen::VECTOR_TYPE, memoryEigen::SCALAR_TYPE, memoryEigen::NA_TYPE};
  op_list_[SCALAR_BROADCAST_OP_] = (&instruction::ExecuteScalarBroadcastOp);

  op_mem_types_[VECTOR_RECIPROCAL_OP_] = {
      memoryEigen::VECTOR_TYPE, memoryEigen::VECTOR_TYPE, memoryEigen::NA_TYPE};
  op_list_[VECTOR_RECIPROCAL_OP_] = (&instruction::ExecuteVectorReciprocalOp);

  op_mem_types_[VECTOR_NORM_OP_] = {
      memoryEigen::SCALAR_TYPE, memoryEigen::VECTOR_TYPE, memoryEigen::NA_TYPE};
  op_list_[VECTOR_NORM_OP_] = (&instruction::ExecuteVectorNormOp);

  op_mem_types_[VECTOR_ABS_OP_] = {
      memoryEigen::VECTOR_TYPE, memoryEigen::VECTOR_TYPE, memoryEigen::NA_TYPE};
  op_list_[VECTOR_ABS_OP_] = (&instruction::ExecuteVectorAbsOp);

  op_mem_types_[VECTOR_SUM_OP_] = {memoryEigen::VECTOR_TYPE,
                                   memoryEigen::VECTOR_TYPE,
                                   memoryEigen::VECTOR_TYPE};
  op_list_[VECTOR_SUM_OP_] = (&instruction::ExecuteVectorSumOp);

  op_mem_types_[VECTOR_DIFF_OP_] = {memoryEigen::VECTOR_TYPE,
                                    memoryEigen::VECTOR_TYPE,
                                    memoryEigen::VECTOR_TYPE};
  op_list_[VECTOR_DIFF_OP_] = (&instruction::ExecuteVectorDiffOp);

  op_mem_types_[VECTOR_PRODUCT_OP_] = {memoryEigen::VECTOR_TYPE,
                                       memoryEigen::VECTOR_TYPE,
                                       memoryEigen::VECTOR_TYPE};
  op_list_[VECTOR_PRODUCT_OP_] = (&instruction::ExecuteVectorProductOp);

  op_mem_types_[VECTOR_DIVISION_OP_] = {memoryEigen::VECTOR_TYPE,
                                        memoryEigen::VECTOR_TYPE,
                                        memoryEigen::VECTOR_TYPE};
  op_list_[VECTOR_DIVISION_OP_] = (&instruction::ExecuteVectorDivisionOp);

  op_mem_types_[VECTOR_INNER_PRODUCT_OP_] = {memoryEigen::SCALAR_TYPE,
                                             memoryEigen::VECTOR_TYPE,
                                             memoryEigen::VECTOR_TYPE};
  op_list_[VECTOR_INNER_PRODUCT_OP_] =
      (&instruction::ExecuteVectorInnerProductOp);

  op_mem_types_[VECTOR_OUTER_PRODUCT_OP_] = {memoryEigen::MATRIX_TYPE,
                                             memoryEigen::VECTOR_TYPE,
                                             memoryEigen::VECTOR_TYPE};
  op_list_[VECTOR_OUTER_PRODUCT_OP_] =
      (&instruction::ExecuteVectorOuterProductOp);

  op_mem_types_[SCALAR_MATRIX_PRODUCT_OP_] = {memoryEigen::MATRIX_TYPE,
                                              memoryEigen::SCALAR_TYPE,
                                              memoryEigen::MATRIX_TYPE};
  op_list_[SCALAR_MATRIX_PRODUCT_OP_] =
      (&instruction::ExecuteScalarMatrixProductOp);

  op_mem_types_[MATRIX_RECIPROCAL_OP_] = {
      memoryEigen::MATRIX_TYPE, memoryEigen::MATRIX_TYPE, memoryEigen::NA_TYPE};
  op_list_[MATRIX_RECIPROCAL_OP_] = (&instruction::ExecuteMatrixReciprocalOp);

  op_mem_types_[MATRIX_VECTOR_PRODUCT_OP_] = {memoryEigen::VECTOR_TYPE,
                                              memoryEigen::MATRIX_TYPE,
                                              memoryEigen::VECTOR_TYPE};
  op_list_[MATRIX_VECTOR_PRODUCT_OP_] =
      (&instruction::ExecuteMatrixVectorProductOp);

  op_mem_types_[VECTOR_COLUMN_BROADCAST_OP_] = {
      memoryEigen::MATRIX_TYPE, memoryEigen::VECTOR_TYPE, memoryEigen::NA_TYPE};
  op_list_[VECTOR_COLUMN_BROADCAST_OP_] =
      (&instruction::ExecuteVectorColumnBroadcastOp);

  op_mem_types_[VECTOR_ROW_BROADCAST_OP_] = {
      memoryEigen::MATRIX_TYPE, memoryEigen::VECTOR_TYPE, memoryEigen::NA_TYPE};
  op_list_[VECTOR_ROW_BROADCAST_OP_] =
      (&instruction::ExecuteVectorRowBroadcastOp);

  op_mem_types_[MATRIX_NORM_OP_] = {
      memoryEigen::SCALAR_TYPE, memoryEigen::MATRIX_TYPE, memoryEigen::NA_TYPE};
  op_list_[MATRIX_NORM_OP_] = (&instruction::ExecuteMatrixNormOp);

  op_mem_types_[MATRIX_COLUMN_NORM_OP_] = {
      memoryEigen::VECTOR_TYPE, memoryEigen::MATRIX_TYPE, memoryEigen::NA_TYPE};
  op_list_[MATRIX_COLUMN_NORM_OP_] = (&instruction::ExecuteMatrixColumnNormOp);

  op_mem_types_[MATRIX_ROW_NORM_OP_] = {
      memoryEigen::VECTOR_TYPE, memoryEigen::MATRIX_TYPE, memoryEigen::NA_TYPE};
  op_list_[MATRIX_ROW_NORM_OP_] = (&instruction::ExecuteMatrixRowNormOp);

  op_mem_types_[MATRIX_TRANSPOSE_OP_] = {
      memoryEigen::MATRIX_TYPE, memoryEigen::MATRIX_TYPE, memoryEigen::NA_TYPE};
  op_list_[MATRIX_TRANSPOSE_OP_] = (&instruction::ExecuteMatrixTransposeOp);

  op_mem_types_[MATRIX_ABS_OP_] = {
      memoryEigen::MATRIX_TYPE, memoryEigen::MATRIX_TYPE, memoryEigen::NA_TYPE};
  op_list_[MATRIX_ABS_OP_] = (&instruction::ExecuteMatrixAbsOp);

  op_mem_types_[MATRIX_SUM_OP_] = {memoryEigen::MATRIX_TYPE,
                                   memoryEigen::MATRIX_TYPE,
                                   memoryEigen::MATRIX_TYPE};
  op_list_[MATRIX_SUM_OP_] = (&instruction::ExecuteMatrixSumOp);

  op_mem_types_[MATRIX_DIFF_OP_] = {memoryEigen::MATRIX_TYPE,
                                    memoryEigen::MATRIX_TYPE,
                                    memoryEigen::MATRIX_TYPE};
  op_list_[MATRIX_DIFF_OP_] = (&instruction::ExecuteMatrixDiffOp);

  op_mem_types_[MATRIX_PRODUCT_OP_] = {memoryEigen::MATRIX_TYPE,
                                       memoryEigen::MATRIX_TYPE,
                                       memoryEigen::MATRIX_TYPE};
  op_list_[MATRIX_PRODUCT_OP_] = (&instruction::ExecuteMatrixProductOp);

  op_mem_types_[MATRIX_DIVISION_OP_] = {memoryEigen::MATRIX_TYPE,
                                        memoryEigen::MATRIX_TYPE,
                                        memoryEigen::MATRIX_TYPE};
  op_list_[MATRIX_DIVISION_OP_] = (&instruction::ExecuteMatrixDivisionOp);

  op_mem_types_[MATRIX_MATRIX_PRODUCT_OP_] = {memoryEigen::MATRIX_TYPE,
                                              memoryEigen::MATRIX_TYPE,
                                              memoryEigen::MATRIX_TYPE};
  op_list_[MATRIX_MATRIX_PRODUCT_OP_] =
      (&instruction::ExecuteMatrixMatrixProductOp);

  op_mem_types_[SCALAR_MIN_OP_] = {memoryEigen::SCALAR_TYPE,
                                   memoryEigen::SCALAR_TYPE,
                                   memoryEigen::SCALAR_TYPE};
  op_list_[SCALAR_MIN_OP_] = (&instruction::ExecuteScalarMinOp);

  op_mem_types_[VECTOR_MIN_OP_] = {memoryEigen::VECTOR_TYPE,
                                   memoryEigen::VECTOR_TYPE,
                                   memoryEigen::VECTOR_TYPE};
  op_list_[VECTOR_MIN_OP_] = (&instruction::ExecuteVectorMinOp);

  op_mem_types_[MATRIX_MIN_OP_] = {memoryEigen::MATRIX_TYPE,
                                   memoryEigen::MATRIX_TYPE,
                                   memoryEigen::MATRIX_TYPE};
  op_list_[MATRIX_MIN_OP_] = (&instruction::ExecuteMatrixMinOp);

  op_mem_types_[SCALAR_MAX_OP_] = {memoryEigen::SCALAR_TYPE,
                                   memoryEigen::SCALAR_TYPE,
                                   memoryEigen::SCALAR_TYPE};
  op_list_[SCALAR_MAX_OP_] = (&instruction::ExecuteScalarMaxOp);

  op_mem_types_[VECTOR_MAX_OP_] = {memoryEigen::VECTOR_TYPE,
                                   memoryEigen::VECTOR_TYPE,
                                   memoryEigen::VECTOR_TYPE};
  op_list_[VECTOR_MAX_OP_] = (&instruction::ExecuteVectorMaxOp);

  op_mem_types_[MATRIX_MAX_OP_] = {memoryEigen::MATRIX_TYPE,
                                   memoryEigen::MATRIX_TYPE,
                                   memoryEigen::MATRIX_TYPE};
  op_list_[MATRIX_MAX_OP_] = (&instruction::ExecuteMatrixMaxOp);

  op_mem_types_[VECTOR_MEAN_OP_] = {
      memoryEigen::SCALAR_TYPE, memoryEigen::VECTOR_TYPE, memoryEigen::NA_TYPE};
  op_list_[VECTOR_MEAN_OP_] = (&instruction::ExecuteVectorMeanOp);

  op_mem_types_[MATRIX_MEAN_OP_] = {
      memoryEigen::SCALAR_TYPE, memoryEigen::MATRIX_TYPE, memoryEigen::NA_TYPE};
  op_list_[MATRIX_MEAN_OP_] = (&instruction::ExecuteMatrixMeanOp);

  op_mem_types_[MATRIX_ROW_MEAN_OP_] = {
      memoryEigen::VECTOR_TYPE, memoryEigen::MATRIX_TYPE, memoryEigen::NA_TYPE};
  op_list_[MATRIX_ROW_MEAN_OP_] = (&instruction::ExecuteMatrixRowMeanOp);

  op_mem_types_[MATRIX_ROW_ST_DEV_OP_] = {
      memoryEigen::VECTOR_TYPE, memoryEigen::MATRIX_TYPE, memoryEigen::NA_TYPE};
  op_list_[MATRIX_ROW_ST_DEV_OP_] = (&instruction::ExecuteMatrixRowStDevOp);

  op_mem_types_[VECTOR_ST_DEV_OP_] = {
      memoryEigen::SCALAR_TYPE, memoryEigen::MATRIX_TYPE, memoryEigen::NA_TYPE};
  op_list_[VECTOR_ST_DEV_OP_] = (&instruction::ExecuteVectorStDevOp);

  op_mem_types_[MATRIX_ST_DEV_OP_] = {
      memoryEigen::SCALAR_TYPE, memoryEigen::MATRIX_TYPE, memoryEigen::NA_TYPE};
  op_list_[MATRIX_ST_DEV_OP_] = (&instruction::ExecuteMatrixStDevOp);

  op_mem_types_[SCALAR_CONST_SET_OP_] = {
      memoryEigen::SCALAR_TYPE, memoryEigen::SCALAR_TYPE, memoryEigen::NA_TYPE};
  op_list_[SCALAR_CONST_SET_OP_] = (&instruction::ExecuteScalarConstSetOp);

  op_mem_types_[VECTOR_CONST_SET_OP_] = {
      memoryEigen::VECTOR_TYPE, memoryEigen::VECTOR_TYPE, memoryEigen::NA_TYPE};
  op_list_[VECTOR_CONST_SET_OP_] = (&instruction::ExecuteVectorConstSetOp);

  op_mem_types_[MATRIX_CONST_SET_OP_] = {
      memoryEigen::MATRIX_TYPE, memoryEigen::MATRIX_TYPE, memoryEigen::NA_TYPE};
  op_list_[MATRIX_CONST_SET_OP_] = (&instruction::ExecuteMatrixConstSetOp);

  op_mem_types_[SCALAR_UNIFORM_SET_OP_] = {memoryEigen::SCALAR_TYPE,
                                           memoryEigen::SCALAR_TYPE,
                                           memoryEigen::SCALAR_TYPE};
  op_list_[SCALAR_UNIFORM_SET_OP_] = (&instruction::ExecuteScalarUniformSetOp);

  op_mem_types_[VECTOR_UNIFORM_SET_OP_] = {memoryEigen::VECTOR_TYPE,
                                           memoryEigen::SCALAR_TYPE,
                                           memoryEigen::SCALAR_TYPE};
  op_list_[VECTOR_UNIFORM_SET_OP_] = (&instruction::ExecuteVectorUniformSetOp);

  op_mem_types_[MATRIX_UNIFORM_SET_OP_] = {memoryEigen::MATRIX_TYPE,
                                           memoryEigen::SCALAR_TYPE,
                                           memoryEigen::SCALAR_TYPE};
  op_list_[MATRIX_UNIFORM_SET_OP_] = (&instruction::ExecuteMatrixUniformSetOp);

  op_mem_types_[SCALAR_GAUSSIAN_SET_OP_] = {memoryEigen::SCALAR_TYPE,
                                            memoryEigen::SCALAR_TYPE,
                                            memoryEigen::SCALAR_TYPE};
  op_list_[SCALAR_GAUSSIAN_SET_OP_] =
      (&instruction::ExecuteScalarGaussianSetOp);

  op_mem_types_[VECTOR_GAUSSIAN_SET_OP_] = {memoryEigen::VECTOR_TYPE,
                                            memoryEigen::SCALAR_TYPE,
                                            memoryEigen::SCALAR_TYPE};
  op_list_[VECTOR_GAUSSIAN_SET_OP_] =
      (&instruction::ExecuteVectorGaussianSetOp);

  op_mem_types_[MATRIX_GAUSSIAN_SET_OP_] = {memoryEigen::MATRIX_TYPE,
                                            memoryEigen::SCALAR_TYPE,
                                            memoryEigen::SCALAR_TYPE};
  op_list_[MATRIX_GAUSSIAN_SET_OP_] =
      (&instruction::ExecuteMatrixGaussianSetOp);

  op_mem_types_[SCALAR_CONDITIONAL_OP_] = {memoryEigen::SCALAR_TYPE,
                                           memoryEigen::SCALAR_TYPE,
                                           memoryEigen::SCALAR_TYPE};
  op_list_[SCALAR_CONDITIONAL_OP_] = (&instruction::ExecuteScalarConditionalOp);

  op_mem_types_[SCALAR_POW_OP_] = {memoryEigen::SCALAR_TYPE,
                                   memoryEigen::SCALAR_TYPE,
                                   memoryEigen::SCALAR_TYPE};
  op_list_[SCALAR_POW_OP_] = (&instruction::ExecuteScalarPowOp);

  op_mem_types_[SCALAR_SQR_OP_] = {
      memoryEigen::SCALAR_TYPE, memoryEigen::SCALAR_TYPE, memoryEigen::NA_TYPE};
  op_list_[SCALAR_SQR_OP_] = (&instruction::ExecuteScalarSqrOp);

  op_mem_types_[SCALAR_CUBE_OP_] = {
      memoryEigen::SCALAR_TYPE, memoryEigen::SCALAR_TYPE, memoryEigen::NA_TYPE};
  op_list_[SCALAR_CUBE_OP_] = (&instruction::ExecuteScalarCubeOp);

  op_mem_types_[SCALAR_TANH_OP_] = {
      memoryEigen::SCALAR_TYPE, memoryEigen::SCALAR_TYPE, memoryEigen::NA_TYPE};
  op_list_[SCALAR_TANH_OP_] = (&instruction::ExecuteScalarTanhOp);

  op_mem_types_[SCALAR_SQRT_OP_] = {
      memoryEigen::SCALAR_TYPE, memoryEigen::SCALAR_TYPE, memoryEigen::NA_TYPE};
  op_list_[SCALAR_SQRT_OP_] = (&instruction::ExecuteScalarSqrtOp);

  op_mem_types_[SCALAR_VECTOR_ASSIGN_OP_] = {
      memoryEigen::SCALAR_TYPE, memoryEigen::VECTOR_TYPE, memoryEigen::NA_TYPE};
  op_list_[SCALAR_VECTOR_ASSIGN_OP_] =
      (&instruction::ExecuteScalarVectorAssignOp);

  op_mem_types_[SCALAR_MATRIX_ASSIGN_OP_] = {
      memoryEigen::SCALAR_TYPE, memoryEigen::MATRIX_TYPE, memoryEigen::NA_TYPE};
  op_list_[SCALAR_MATRIX_ASSIGN_OP_] =
      (&instruction::ExecuteScalarMatrixAssignOp);

  op_mem_types_[OBS_BUFF_SLICE_OP_] =
      {memoryEigen::VECTOR_TYPE, memoryEigen::VECTOR_TYPE,
       memoryEigen::NA_TYPE};
  op_list_[OBS_BUFF_SLICE_OP_] = {&instruction::ExecuteObsBuffSliceOp};
}

// constructor
instruction::instruction(std::unordered_map<string, std::any> &params,
                         mt19937 &rng) {
  memory_size_ = std::any_cast<int>(params["memory_size"]);
  memIndices_ = std::any_cast<int>(params["memory_indices"]);
  rng_ = rng;
}

// copy construction
instruction::instruction(instruction &i) {
  // TODO(skelly): check which things actually need to be copied
  memory_size_ = i.memory_size_;
  memIndices_ = i.memIndices_;
  out_ = i.out_;
  in1_ = i.in1_;
  in2_ = i.in2_;
  op_mem_types_ = i.op_mem_types_;
  op_list_ = i.op_list_;

  in1Src_ = i.in1Src_;
  in2Src_ = i.in2Src_;
  outIdx_ = i.outIdx_;
  op_ = i.op_;
  in0Idx_ = i.in0Idx_;
  in1Idx_ = i.in1Idx_;
  in2Idx_ = i.in2Idx_;
  in3Idx_ = i.in3Idx_;

  rng_ = i.rng_;
}

// // Protect input indices from ranges larger than memory data structures.
// void instruction::BoundMemoryIndices(int observation_buff_size) {   
//   for (int in = 0; in < 2; in++) {
//     if (IsObs(in)) {
//       SetInIdx(in, GetInIdx(in) % max(1, observation_buff_size - 1));
//     } else {
//       SetInIdx(in, GetInIdx(in) % memIndices_);
//     }
//   }
//   memory_size_ = observation_buff_size;
//   in2Idx_ = in2Idx_ % (max(1, memory_size_ - 1));
//   in3Idx_ = in3Idx_ % (max(1, memory_size_ - 1));
// }

void instruction::Mutate(bool randomize, vector<bool> &legal_ops,
                         int observation_buff_size, mt19937 &rng) {
  const int max_index = 100; 
  auto dis_index = std::uniform_int_distribution<>(0, max_index);                         
  if (randomize) {  // Randomly set each part of this instruction.
    std::uniform_int_distribution<> dis(0, 1);
    in1Src_ = dis(rng);
    in2Src_ = dis(rng);

    dis = std::uniform_int_distribution<>(0, legal_ops.size() - 1);
    do {
      op_ = dis(rng);
    } while (!legal_ops[op_]);

    //TODO(skelly): use MutateInt()
    outIdx_ = dis_index(rng);
    in0Idx_ = dis_index(rng);
    in1Idx_ = dis_index(rng);
    in2Idx_ = dis_index(rng);
    in3Idx_ = dis_index(rng);

  } else {  // Randomly change one part of this instruction.
    std::uniform_int_distribution<> dis(0, 5);
    int i = dis(rng);
    if (i == 0) {  // Change in1 src to private memory or observation.
      MutateInt(in1Src_, 0, 1, rng);
    } else if (i == 1) {  // Change in2 src to private memory or observation.
      MutateInt(in2Src_, 0, 1, rng);
    } else if (i == 2) {  // Change out index.
      MutateInt(outIdx_, 0, max_index, rng);
    } else if (i == 3) {  // Change operation.
      do {
        MutateInt(op_, 0, int(legal_ops.size() - 1), rng);
      } while (!legal_ops[op_]);
    } else if (i == 4) {  // Change in1 index.
      MutateInt(in0Idx_, 0, max_index, rng);
    } else if (i == 5) {  // Change in2 index.
      MutateInt(in1Idx_, 0, max_index, rng);
    }
    // else if (i == 6) {  // Change in3 index. Used as index to vector or matrix
    //                       // memory.
    //   MutateInt(in2Idx_, 0, max_index, rng);
    // } else if (i == 7) {  // Change in4 index. Used as index to vector or matrix
    //                       // memory.
    //   MutateInt(in3Idx_, 0, max_index, rng);
    // }
  }
//   BoundMemoryIndices(observation_buff_size);
//   if (op_ == OBS_BUFF_SLICE_OP_) in1Src_  = 1;
}
