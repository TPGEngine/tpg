#include "instruction.h"

std::vector<std::vector<size_t> > instruction::op_signatures_(NUM_OP);
std::vector<instruction::operation> instruction::op_functions_(NUM_OP);
std::vector<std::string> instruction::op_names_(NUM_OP);

string instruction::ToString() {
  ostringstream oss;
  oss << in1Src_ << "_";
  oss << in2Src_ << "_";
  oss << outIdx_ << "_";
  oss << op_ << "_";
  oss << in0Idx_ << "_";
  oss << in1Idx_ << "_";
  oss << in2Idx_ << "_";
  oss << in3Idx_ << "_";
  return oss.str();
}

void instruction::SetupOps() {
  op_signatures_[SCALAR_SUM_OP_] = {MemoryEigen::kScalarType_,
                                   MemoryEigen::kScalarType_,
                                   MemoryEigen::kScalarType_};
  op_functions_[SCALAR_SUM_OP_] = (&instruction::ExecuteScalarSumOp);
  op_names_[SCALAR_SUM_OP_] = "SCALAR_SUM_OP";

  op_signatures_[SCALAR_DIFF_OP_] = {MemoryEigen::kScalarType_,
                                    MemoryEigen::kScalarType_,
                                    MemoryEigen::kScalarType_};
  op_functions_[SCALAR_DIFF_OP_] = (&instruction::ExecuteScalarDiffOp);
  op_names_[SCALAR_DIFF_OP_] = "SCALAR_DIFF_OP"; 

  op_signatures_[SCALAR_PRODUCT_OP_] = {MemoryEigen::kScalarType_,
                                       MemoryEigen::kScalarType_,
                                       MemoryEigen::kScalarType_};
  op_functions_[SCALAR_PRODUCT_OP_] = (&instruction::ExecuteScalarProductOp);
  op_names_[SCALAR_PRODUCT_OP_] = "SCALAR_PRODUCT_OP";

  op_signatures_[SCALAR_DIVISION_OP_] = {MemoryEigen::kScalarType_,
                                        MemoryEigen::kScalarType_,
                                        MemoryEigen::kScalarType_};
  op_functions_[SCALAR_DIVISION_OP_] = (&instruction::ExecuteScalarDivisionOp);
  op_names_[SCALAR_DIVISION_OP_] = "SCALAR_DIVISION_OP";

  op_signatures_[SCALAR_ABS_OP_] = {
      MemoryEigen::kScalarType_, MemoryEigen::kScalarType_};
  op_functions_[SCALAR_ABS_OP_] = (&instruction::ExecuteScalarAbsOp);
  op_names_[SCALAR_ABS_OP_] = "SCALAR_ABS_OP";

  op_signatures_[SCALAR_SIN_OP_] = {
      MemoryEigen::kScalarType_, MemoryEigen::kScalarType_};
  op_functions_[SCALAR_SIN_OP_] = (&instruction::ExecuteScalarSinOp);
  op_names_[SCALAR_SIN_OP_] = "SCALAR_SIN_OP";

  op_signatures_[SCALAR_COS_OP_] = {
      MemoryEigen::kScalarType_, MemoryEigen::kScalarType_};
  op_functions_[SCALAR_COS_OP_] = (&instruction::ExecuteScalarCosOp);
  op_names_[SCALAR_COS_OP_] = "SCALAR_COS_OP";

  op_signatures_[SCALAR_TAN_OP_] = {
      MemoryEigen::kScalarType_, MemoryEigen::kScalarType_};
  op_functions_[SCALAR_TAN_OP_] = (&instruction::ExecuteScalarTanOp);
  op_names_[SCALAR_TAN_OP_] = "SCALAR_TAN_OP";

  op_signatures_[SCALAR_EXP_OP_] = {
      MemoryEigen::kScalarType_, MemoryEigen::kScalarType_};
  op_functions_[SCALAR_EXP_OP_] = (&instruction::ExecuteScalarExpOp);
  op_names_[SCALAR_EXP_OP_] = "SCALAR_EXP_OP";

  op_signatures_[SCALAR_LOG_OP_] = {
      MemoryEigen::kScalarType_, MemoryEigen::kScalarType_};
  op_functions_[SCALAR_LOG_OP_] = (&instruction::ExecuteScalarLogOp);
  op_names_[SCALAR_LOG_OP_] = "SCALAR_LOG_OP";

  op_signatures_[SCALAR_RECIPROCAL_OP_] = {
      MemoryEigen::kScalarType_, MemoryEigen::kScalarType_};
  op_functions_[SCALAR_RECIPROCAL_OP_] = (&instruction::ExecuteScalarReciprocalOp);
  op_names_[SCALAR_RECIPROCAL_OP_] = "SCALAR_RECIPROCAL_OP";

  op_signatures_[SCALAR_SIN_OP_] = {
      MemoryEigen::kScalarType_, MemoryEigen::kScalarType_};
  op_functions_[SCALAR_SIN_OP_] = (&instruction::ExecuteScalarSinOp);
  op_names_[SCALAR_SIN_OP_] = "SCALAR_SIN_OP";

  op_signatures_[SCALAR_COS_OP_] = {
      MemoryEigen::kScalarType_, MemoryEigen::kScalarType_};
  op_functions_[SCALAR_COS_OP_] = (&instruction::ExecuteScalarCosOp);
  op_names_[SCALAR_COS_OP_] = "SCALAR_COS_OP";

  op_signatures_[SCALAR_ARCSIN_OP_] = {
      MemoryEigen::kScalarType_, MemoryEigen::kScalarType_};
  op_functions_[SCALAR_ARCSIN_OP_] = (&instruction::ExecuteScalarArcSinOp);
  op_names_[SCALAR_ARCSIN_OP_] = "SCALAR_ARCSIN_OP";

  op_signatures_[SCALAR_ARCCOS_OP_] = {
      MemoryEigen::kScalarType_, MemoryEigen::kScalarType_};
  op_functions_[SCALAR_ARCCOS_OP_] = (&instruction::ExecuteScalarArcCosOp);
  op_names_[SCALAR_ARCCOS_OP_] = "SCALAR_ARCCOS_OP";

  op_signatures_[SCALAR_ARCTAN_OP_] = {
      MemoryEigen::kScalarType_, MemoryEigen::kScalarType_};
  op_functions_[SCALAR_ARCTAN_OP_] = (&instruction::ExecuteScalarArcTanOp);
  op_names_[SCALAR_ARCTAN_OP_] = "SCALAR_ARCTAN_OP";

  op_signatures_[SCALAR_HEAVYSIDE_OP_] = {
      MemoryEigen::kScalarType_, MemoryEigen::kScalarType_};
  op_functions_[SCALAR_HEAVYSIDE_OP_] = (&instruction::ExecuteScalarHeavisideOp);
  op_names_[SCALAR_HEAVYSIDE_OP_] = "SCALAR_HEAVYSIDE_OP";

  op_signatures_[VECTOR_HEAVYSIDE_OP_] = {
      MemoryEigen::kVectorType_, MemoryEigen::kVectorType_};
  op_functions_[VECTOR_HEAVYSIDE_OP_] = (&instruction::ExecuteVectorHeavisideOp);
  op_names_[VECTOR_HEAVYSIDE_OP_] = "VECTOR_HEAVYSIDE_OP";

  op_signatures_[MATRIX_HEAVYSIDE_OP_] = {
      MemoryEigen::kMatrixType_, MemoryEigen::kMatrixType_};
  op_functions_[MATRIX_HEAVYSIDE_OP_] = (&instruction::ExecuteMatrixHeavisideOp);
  op_names_[MATRIX_HEAVYSIDE_OP_] = "MATRIX_HEAVYSIDE_OP";

  op_signatures_[SCALAR_VECTOR_PRODUCT_OP_] = {MemoryEigen::kVectorType_,
                                              MemoryEigen::kScalarType_,
                                              MemoryEigen::kVectorType_};
  op_functions_[SCALAR_VECTOR_PRODUCT_OP_] =
      (&instruction::ExecuteScalarVectorProductOp);
  op_names_[SCALAR_VECTOR_PRODUCT_OP_] = "SCALAR_VECTOR_PRODUCT_OP";    

  op_signatures_[SCALAR_BROADCAST_OP_] = {
      MemoryEigen::kVectorType_, MemoryEigen::kScalarType_};
  op_functions_[SCALAR_BROADCAST_OP_] = (&instruction::ExecuteScalarBroadcastOp);
  op_names_[SCALAR_BROADCAST_OP_] = "SCALAR_BROADCAST_OP";

  op_signatures_[VECTOR_RECIPROCAL_OP_] = {
      MemoryEigen::kVectorType_, MemoryEigen::kVectorType_};
  op_functions_[VECTOR_RECIPROCAL_OP_] = (&instruction::ExecuteVectorReciprocalOp);
  op_names_[VECTOR_RECIPROCAL_OP_] = "VECTOR_RECIPROCAL_OP";

  op_signatures_[VECTOR_NORM_OP_] = {
      MemoryEigen::kScalarType_, MemoryEigen::kVectorType_};
  op_functions_[VECTOR_NORM_OP_] = (&instruction::ExecuteVectorNormOp);
  op_names_[VECTOR_NORM_OP_] = "VECTOR_NORM_OP";

  op_signatures_[VECTOR_ABS_OP_] = {
      MemoryEigen::kVectorType_, MemoryEigen::kVectorType_};
  op_functions_[VECTOR_ABS_OP_] = (&instruction::ExecuteVectorAbsOp);
  op_names_[VECTOR_ABS_OP_] = "VECTOR_ABS_OP";

  op_signatures_[VECTOR_SUM_OP_] = {MemoryEigen::kVectorType_,
                                   MemoryEigen::kVectorType_,
                                   MemoryEigen::kVectorType_};
  op_functions_[VECTOR_SUM_OP_] = (&instruction::ExecuteVectorSumOp);
  op_names_[VECTOR_SUM_OP_] = "VECTOR_SUM_OP";

  op_signatures_[VECTOR_DIFF_OP_] = {MemoryEigen::kVectorType_,
                                    MemoryEigen::kVectorType_,
                                    MemoryEigen::kVectorType_};
  op_functions_[VECTOR_DIFF_OP_] = (&instruction::ExecuteVectorDiffOp);
  op_names_[VECTOR_DIFF_OP_] = "VECTOR_DIFF_OP";

  op_signatures_[VECTOR_PRODUCT_OP_] = {MemoryEigen::kVectorType_,
                                       MemoryEigen::kVectorType_,
                                       MemoryEigen::kVectorType_};
  op_functions_[VECTOR_PRODUCT_OP_] = (&instruction::ExecuteVectorProductOp);
  op_names_[VECTOR_PRODUCT_OP_] = "VECTOR_PRODUCT_OP";

  op_signatures_[VECTOR_DIVISION_OP_] = {MemoryEigen::kVectorType_,
                                        MemoryEigen::kVectorType_,
                                        MemoryEigen::kVectorType_};
  op_functions_[VECTOR_DIVISION_OP_] = (&instruction::ExecuteVectorDivisionOp);
  op_names_[VECTOR_DIVISION_OP_] = "VECTOR_DIVISION_OP";

  op_signatures_[VECTOR_INNER_PRODUCT_OP_] = {MemoryEigen::kScalarType_,
                                             MemoryEigen::kVectorType_,
                                             MemoryEigen::kVectorType_};
  op_functions_[VECTOR_INNER_PRODUCT_OP_] =
      (&instruction::ExecuteVectorInnerProductOp);
  op_names_[VECTOR_INNER_PRODUCT_OP_] = "VECTOR_INNER_PRODUCT_OP";    

  op_signatures_[VECTOR_OUTER_PRODUCT_OP_] = {MemoryEigen::kMatrixType_,
                                             MemoryEigen::kVectorType_,
                                             MemoryEigen::kVectorType_};
  op_functions_[VECTOR_OUTER_PRODUCT_OP_] =
      (&instruction::ExecuteVectorOuterProductOp);
  op_names_[VECTOR_OUTER_PRODUCT_OP_] = "VECTOR_OUTER_PRODUCT_OP";    

  op_signatures_[SCALAR_MATRIX_PRODUCT_OP_] = {MemoryEigen::kMatrixType_,
                                              MemoryEigen::kScalarType_,
                                              MemoryEigen::kMatrixType_};
  op_functions_[SCALAR_MATRIX_PRODUCT_OP_] =
      (&instruction::ExecuteScalarMatrixProductOp);
  op_names_[SCALAR_MATRIX_PRODUCT_OP_] = "SCALAR_MATRIX_PRODUCT_OP";    

  op_signatures_[MATRIX_RECIPROCAL_OP_] = {
      MemoryEigen::kMatrixType_, MemoryEigen::kMatrixType_};
  op_functions_[MATRIX_RECIPROCAL_OP_] = (&instruction::ExecuteMatrixReciprocalOp);
  op_names_[MATRIX_RECIPROCAL_OP_] = "MATRIX_RECIPROCAL_OP";

  op_signatures_[MATRIX_VECTOR_PRODUCT_OP_] = {MemoryEigen::kVectorType_,
                                              MemoryEigen::kMatrixType_,
                                              MemoryEigen::kVectorType_};
  op_functions_[MATRIX_VECTOR_PRODUCT_OP_] =
      (&instruction::ExecuteMatrixVectorProductOp);
  op_names_[MATRIX_VECTOR_PRODUCT_OP_] = "MATRIX_VECTOR_PRODUCT_OP";    

  op_signatures_[VECTOR_COLUMN_BROADCAST_OP_] = {
      MemoryEigen::kMatrixType_, MemoryEigen::kVectorType_};
  op_functions_[VECTOR_COLUMN_BROADCAST_OP_] =
      (&instruction::ExecuteVectorColumnBroadcastOp);
  op_names_[VECTOR_COLUMN_BROADCAST_OP_] = "VECTOR_COLUMN_BROADCAST_OP";    

  op_signatures_[VECTOR_ROW_BROADCAST_OP_] = {
      MemoryEigen::kMatrixType_, MemoryEigen::kVectorType_};
  op_functions_[VECTOR_ROW_BROADCAST_OP_] =
      (&instruction::ExecuteVectorRowBroadcastOp);
  op_names_[VECTOR_ROW_BROADCAST_OP_] = "VECTOR_ROW_BROADCAST_OP";    

  op_signatures_[MATRIX_NORM_OP_] = {
      MemoryEigen::kScalarType_, MemoryEigen::kMatrixType_};
  op_functions_[MATRIX_NORM_OP_] = (&instruction::ExecuteMatrixNormOp);
  op_names_[MATRIX_NORM_OP_] = "MATRIX_NORM_OP";

  op_signatures_[MATRIX_COLUMN_NORM_OP_] = {
      MemoryEigen::kVectorType_, MemoryEigen::kMatrixType_};
  op_functions_[MATRIX_COLUMN_NORM_OP_] = (&instruction::ExecuteMatrixColumnNormOp);
  op_names_[MATRIX_COLUMN_NORM_OP_] = "MATRIX_COLUMN_NORM_OP";

  op_signatures_[MATRIX_ROW_NORM_OP_] = {
      MemoryEigen::kVectorType_, MemoryEigen::kMatrixType_};
  op_functions_[MATRIX_ROW_NORM_OP_] = (&instruction::ExecuteMatrixRowNormOp);
  op_names_[MATRIX_ROW_NORM_OP_] = "MATRIX_ROW_NORM_OP";

  op_signatures_[MATRIX_TRANSPOSE_OP_] = {
      MemoryEigen::kMatrixType_, MemoryEigen::kMatrixType_};
  op_functions_[MATRIX_TRANSPOSE_OP_] = (&instruction::ExecuteMatrixTransposeOp);
  op_names_[MATRIX_TRANSPOSE_OP_] = "MATRIX_TRANSPOSE_OP";

  op_signatures_[MATRIX_ABS_OP_] = {
      MemoryEigen::kMatrixType_, MemoryEigen::kMatrixType_};
  op_functions_[MATRIX_ABS_OP_] = (&instruction::ExecuteMatrixAbsOp);
  op_names_[MATRIX_ABS_OP_] = "MATRIX_ABS_OP";

  op_signatures_[MATRIX_SUM_OP_] = {MemoryEigen::kMatrixType_,
                                   MemoryEigen::kMatrixType_,
                                   MemoryEigen::kMatrixType_};
  op_functions_[MATRIX_SUM_OP_] = (&instruction::ExecuteMatrixSumOp);
  op_names_[MATRIX_SUM_OP_] = "MATRIX_SUM_OP";

  op_signatures_[MATRIX_DIFF_OP_] = {MemoryEigen::kMatrixType_,
                                    MemoryEigen::kMatrixType_,
                                    MemoryEigen::kMatrixType_};
  op_functions_[MATRIX_DIFF_OP_] = (&instruction::ExecuteMatrixDiffOp);
  op_names_[MATRIX_DIFF_OP_] = "MATRIX_DIFF_OP";

  op_signatures_[MATRIX_PRODUCT_OP_] = {MemoryEigen::kMatrixType_,
                                       MemoryEigen::kMatrixType_,
                                       MemoryEigen::kMatrixType_};
  op_functions_[MATRIX_PRODUCT_OP_] = (&instruction::ExecuteMatrixProductOp);
  op_names_[MATRIX_PRODUCT_OP_] = "MATRIX_PRODUCT_OP";

  op_signatures_[MATRIX_DIVISION_OP_] = {MemoryEigen::kMatrixType_,
                                        MemoryEigen::kMatrixType_,
                                        MemoryEigen::kMatrixType_};
  op_functions_[MATRIX_DIVISION_OP_] = (&instruction::ExecuteMatrixDivisionOp);
  op_names_[MATRIX_DIVISION_OP_] = "MATRIX_DIVISION_OP";

  op_signatures_[MATRIX_MATRIX_PRODUCT_OP_] = {MemoryEigen::kMatrixType_,
                                              MemoryEigen::kMatrixType_,
                                              MemoryEigen::kMatrixType_};
  op_functions_[MATRIX_MATRIX_PRODUCT_OP_] =
      (&instruction::ExecuteMatrixMatrixProductOp);
  op_names_[MATRIX_MATRIX_PRODUCT_OP_] = "MATRIX_MATRIX_PRODUCT_OP";

  op_signatures_[SCALAR_MIN_OP_] = {MemoryEigen::kScalarType_,
                                   MemoryEigen::kScalarType_,
                                   MemoryEigen::kScalarType_};
  op_functions_[SCALAR_MIN_OP_] = (&instruction::ExecuteScalarMinOp);
  op_names_[SCALAR_MIN_OP_] = "SCALAR_MIN_OP";

  op_signatures_[VECTOR_MIN_OP_] = {MemoryEigen::kVectorType_,
                                   MemoryEigen::kVectorType_,
                                   MemoryEigen::kVectorType_};
  op_functions_[VECTOR_MIN_OP_] = (&instruction::ExecuteVectorMinOp);
  op_names_[VECTOR_MIN_OP_] = "VECTOR_MIN_OP";

  op_signatures_[MATRIX_MIN_OP_] = {MemoryEigen::kMatrixType_,
                                   MemoryEigen::kMatrixType_,
                                   MemoryEigen::kMatrixType_};
  op_functions_[MATRIX_MIN_OP_] = (&instruction::ExecuteMatrixMinOp);
  op_names_[MATRIX_MIN_OP_] = "MATRIX_MIN_OP";

  op_signatures_[SCALAR_MAX_OP_] = {MemoryEigen::kScalarType_,
                                   MemoryEigen::kScalarType_,
                                   MemoryEigen::kScalarType_};
  op_functions_[SCALAR_MAX_OP_] = (&instruction::ExecuteScalarMaxOp);
  op_names_[SCALAR_MAX_OP_] = "SCALAR_MAX_OP";

  op_signatures_[VECTOR_MAX_OP_] = {MemoryEigen::kVectorType_,
                                   MemoryEigen::kVectorType_,
                                   MemoryEigen::kVectorType_};
  op_functions_[VECTOR_MAX_OP_] = (&instruction::ExecuteVectorMaxOp);
  op_names_[VECTOR_MAX_OP_] = "VECTOR_MAX_OP";

  op_signatures_[MATRIX_MAX_OP_] = {MemoryEigen::kMatrixType_,
                                   MemoryEigen::kMatrixType_,
                                   MemoryEigen::kMatrixType_};
  op_functions_[MATRIX_MAX_OP_] = (&instruction::ExecuteMatrixMaxOp);
  op_names_[MATRIX_MAX_OP_] = "MATRIX_MAX_OP";

  op_signatures_[VECTOR_MEAN_OP_] = {
      MemoryEigen::kScalarType_, MemoryEigen::kVectorType_};
  op_functions_[VECTOR_MEAN_OP_] = (&instruction::ExecuteVectorMeanOp);
  op_names_[VECTOR_MEAN_OP_] = "VECTOR_MEAN_OP";

  op_signatures_[MATRIX_MEAN_OP_] = {
      MemoryEigen::kScalarType_, MemoryEigen::kMatrixType_};
  op_functions_[MATRIX_MEAN_OP_] = (&instruction::ExecuteMatrixMeanOp);
  op_names_[MATRIX_MEAN_OP_] = "MATRIX_MEAN_OP";

  op_signatures_[MATRIX_ROW_MEAN_OP_] = {
      MemoryEigen::kVectorType_, MemoryEigen::kMatrixType_};
  op_functions_[MATRIX_ROW_MEAN_OP_] = (&instruction::ExecuteMatrixRowMeanOp);
  op_names_[MATRIX_ROW_MEAN_OP_] = "MATRIX_ROW_MEAN_OP";

  op_signatures_[MATRIX_ROW_ST_DEV_OP_] = {
      MemoryEigen::kVectorType_, MemoryEigen::kMatrixType_};
  op_functions_[MATRIX_ROW_ST_DEV_OP_] = (&instruction::ExecuteMatrixRowStDevOp);
  op_names_[MATRIX_ROW_ST_DEV_OP_] = "MATRIX_ROW_ST_DEV_OP";

  op_signatures_[VECTOR_ST_DEV_OP_] = {
      MemoryEigen::kScalarType_, MemoryEigen::kMatrixType_};
  op_functions_[VECTOR_ST_DEV_OP_] = (&instruction::ExecuteVectorStDevOp);
  op_names_[VECTOR_ST_DEV_OP_] = "VECTOR_ST_DEV_OP";

  op_signatures_[MATRIX_ST_DEV_OP_] = {
      MemoryEigen::kScalarType_, MemoryEigen::kMatrixType_};
  op_functions_[MATRIX_ST_DEV_OP_] = (&instruction::ExecuteMatrixStDevOp);
  op_names_[MATRIX_ST_DEV_OP_] = "MATRIX_ST_DEV_OP";

  op_signatures_[SCALAR_CONST_SET_OP_] = {
      MemoryEigen::kScalarType_, MemoryEigen::kScalarType_};
  op_functions_[SCALAR_CONST_SET_OP_] = (&instruction::ExecuteScalarConstSetOp);
  op_names_[SCALAR_CONST_SET_OP_] = "SCALAR_CONST_SET_OP";

  op_signatures_[VECTOR_CONST_SET_OP_] = {
      MemoryEigen::kVectorType_, MemoryEigen::kVectorType_};
  op_functions_[VECTOR_CONST_SET_OP_] = (&instruction::ExecuteVectorConstSetOp);
  op_names_[VECTOR_CONST_SET_OP_] = "VECTOR_CONST_SET_OP";

  op_signatures_[MATRIX_CONST_SET_OP_] = {
      MemoryEigen::kMatrixType_, MemoryEigen::kMatrixType_};
  op_functions_[MATRIX_CONST_SET_OP_] = (&instruction::ExecuteMatrixConstSetOp);
  op_names_[MATRIX_CONST_SET_OP_] = "MATRIX_CONST_SET_OP";

  op_signatures_[SCALAR_UNIFORM_SET_OP_] = {MemoryEigen::kScalarType_,
                                           MemoryEigen::kScalarType_,
                                           MemoryEigen::kScalarType_};
  op_functions_[SCALAR_UNIFORM_SET_OP_] = (&instruction::ExecuteScalarUniformSetOp);
  op_names_[SCALAR_UNIFORM_SET_OP_] = "SCALAR_UNIFORM_SET_OP";

  op_signatures_[VECTOR_UNIFORM_SET_OP_] = {MemoryEigen::kVectorType_,
                                           MemoryEigen::kScalarType_,
                                           MemoryEigen::kScalarType_};
  op_functions_[VECTOR_UNIFORM_SET_OP_] = (&instruction::ExecuteVectorUniformSetOp);
  op_names_[VECTOR_UNIFORM_SET_OP_] = "VECTOR_UNIFORM_SET_OP";

  op_signatures_[MATRIX_UNIFORM_SET_OP_] = {MemoryEigen::kMatrixType_,
                                           MemoryEigen::kScalarType_,
                                           MemoryEigen::kScalarType_};
  op_functions_[MATRIX_UNIFORM_SET_OP_] = (&instruction::ExecuteMatrixUniformSetOp);
  op_names_[MATRIX_UNIFORM_SET_OP_] = "MATRIX_UNIFORM_SET_OP";

  op_signatures_[SCALAR_GAUSSIAN_SET_OP_] = {MemoryEigen::kScalarType_,
                                            MemoryEigen::kScalarType_,
                                            MemoryEigen::kScalarType_};
  op_functions_[SCALAR_GAUSSIAN_SET_OP_] =
      (&instruction::ExecuteScalarGaussianSetOp);
  op_names_[SCALAR_GAUSSIAN_SET_OP_] = "SCALAR_GAUSSIAN_SET_OP";    

  op_signatures_[VECTOR_GAUSSIAN_SET_OP_] = {MemoryEigen::kVectorType_,
                                            MemoryEigen::kScalarType_,
                                            MemoryEigen::kScalarType_};
  op_functions_[VECTOR_GAUSSIAN_SET_OP_] =
      (&instruction::ExecuteVectorGaussianSetOp);
  op_names_[VECTOR_GAUSSIAN_SET_OP_] = "VECTOR_GAUSSIAN_SET_OP";    

  op_signatures_[MATRIX_GAUSSIAN_SET_OP_] = {MemoryEigen::kMatrixType_,
                                            MemoryEigen::kScalarType_,
                                            MemoryEigen::kScalarType_};
  op_functions_[MATRIX_GAUSSIAN_SET_OP_] =
      (&instruction::ExecuteMatrixGaussianSetOp);
  op_names_[MATRIX_GAUSSIAN_SET_OP_] = "MATRIX_GAUSSIAN_SET_OP";    

  op_signatures_[SCALAR_CONDITIONAL_OP_] = {MemoryEigen::kScalarType_,
                                           MemoryEigen::kScalarType_,
                                           MemoryEigen::kScalarType_};
  op_functions_[SCALAR_CONDITIONAL_OP_] = (&instruction::ExecuteScalarConditionalOp);
  op_names_[SCALAR_CONDITIONAL_OP_] = "SCALAR_CONDITIONAL_OP";

  op_signatures_[SCALAR_POW_OP_] = {MemoryEigen::kScalarType_,
                                   MemoryEigen::kScalarType_,
                                   MemoryEigen::kScalarType_};
  op_functions_[SCALAR_POW_OP_] = (&instruction::ExecuteScalarPowOp);
  op_names_[SCALAR_POW_OP_] = "SCALAR_POW_OP";

  op_signatures_[SCALAR_SQR_OP_] = {
      MemoryEigen::kScalarType_, MemoryEigen::kScalarType_};
  op_functions_[SCALAR_SQR_OP_] = (&instruction::ExecuteScalarSqrOp);
  op_names_[SCALAR_SQR_OP_] = "SCALAR_SQR_OP";

  op_signatures_[SCALAR_CUBE_OP_] = {
      MemoryEigen::kScalarType_, MemoryEigen::kScalarType_};
  op_functions_[SCALAR_CUBE_OP_] = (&instruction::ExecuteScalarCubeOp);
  op_names_[SCALAR_CUBE_OP_] = "SCALAR_CUBE_OP";

  op_signatures_[SCALAR_TANH_OP_] = {
      MemoryEigen::kScalarType_, MemoryEigen::kScalarType_};
  op_functions_[SCALAR_TANH_OP_] = (&instruction::ExecuteScalarTanhOp);
  op_names_[SCALAR_TANH_OP_] = "SCALAR_TANH_OP";

  op_signatures_[SCALAR_SQRT_OP_] = {
      MemoryEigen::kScalarType_, MemoryEigen::kScalarType_};
  op_functions_[SCALAR_SQRT_OP_] = (&instruction::ExecuteScalarSqrtOp);
  op_names_[SCALAR_SQRT_OP_] = "SCALAR_SQRT_OP";

  op_signatures_[SCALAR_VECTOR_ASSIGN_OP_] = {
      MemoryEigen::kScalarType_, MemoryEigen::kVectorType_};
  op_functions_[SCALAR_VECTOR_ASSIGN_OP_] =
      (&instruction::ExecuteScalarVectorAssignOp);
  op_names_[SCALAR_VECTOR_ASSIGN_OP_] = "SCALAR_VECTOR_ASSIGN_OP";    

  op_signatures_[SCALAR_MATRIX_ASSIGN_OP_] = {
      MemoryEigen::kScalarType_, MemoryEigen::kMatrixType_};
  op_functions_[SCALAR_MATRIX_ASSIGN_OP_] =
      (&instruction::ExecuteScalarMatrixAssignOp);
  op_names_[SCALAR_MATRIX_ASSIGN_OP_] = "SCALAR_MATRIX_ASSIGN_OP";

  op_signatures_[OBS_BUFF_SLICE_OP_] =
      {MemoryEigen::kVectorType_, MemoryEigen::kVectorType_};
  op_functions_[OBS_BUFF_SLICE_OP_] = {&instruction::ExecuteObsBuffSliceOp};
  op_names_[OBS_BUFF_SLICE_OP_] = "OBS_BUFF_SLICE_OP";
}

// Constructor
instruction::instruction(std::unordered_map<string, std::any> &params,
                         mt19937 &rng) {
  memIndices_ = std::any_cast<int>(params["n_memories"]);
  rng_ = rng;
}

// Copy constructor
instruction::instruction(instruction &i) {
  // TODO(skelly): check which things actually need to be copied
  memIndices_ = i.memIndices_;
  out_ = i.out_;
  in1_ = i.in1_;
  in2_ = i.in2_;
  op_signatures_ = i.op_signatures_;
  op_functions_ = i.op_functions_;

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

void instruction::Mutate(bool randomize, vector<bool> &legal_ops,
                         int observation_buff_size, mt19937 &rng) {
  const int max_index = 1000000;  // TODO(skelly): fix magic
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
    std::uniform_int_distribution<> dis(0, 7);
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
    } else if (i == 4) {  // Change in0 index.
      MutateInt(in0Idx_, 0, max_index, rng);
    } else if (i == 5) {  // Change in1 index.
      MutateInt(in1Idx_, 0, max_index, rng);
    } else if (i == 6) {  // Change in2 index. 
      MutateInt(in2Idx_, 0, max_index, rng);
    } else if (i == 7) {  // Change in3 index. 
      MutateInt(in3Idx_, 0, max_index, rng);
    }
  }
}
