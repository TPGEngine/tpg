#ifndef ActionWrappers_h
#define ActionWrappers_h

#include "EvalData.h"

#include <iostream>
#include <Eigen/Dense>
#include <vector>
#include <algorithm> // For std::transform

// TPG represents discrete actions as negative ints starting at -1
// Map them to positive ints starting at 0
int WrapDiscreteAction(EvalData &eval) {
    return (eval.program_out->action_ * -1) - 1;
}

double WrapContinuousAction(EvalData &eval) {
    return eval.program_out->private_memory_[MemoryEigen::kScalarType_]
        ->working_memory_[1](0, 0);
}

double WrapContinuousActionSigmoid(EvalData &eval) {
    double p = eval.program_out->private_memory_[MemoryEigen::kScalarType_]
                   ->working_memory_[1](0, 0);
    return 1 / (1 + exp(-p));
}

vector<double> WrapVectorAction(EvalData &eval) {
    auto mat = eval.program_out->private_memory_[MemoryEigen::kVectorType_]
                   ->working_memory_[1];
    vector<double> vec(mat.data(), mat.data() + mat.rows() * mat.cols());
    return vec;
}

vector<double> WrapVectorActionSigmoid(EvalData &eval) {
    auto mat = eval.program_out->private_memory_[MemoryEigen::kVectorType_]
                   ->working_memory_[1];
    vector<double> vec(mat.data(), mat.data() + mat.rows() * mat.cols());
    for (auto &v : vec) v = sigmoid(v);  // TODO(skelly): better/faster way?
    return vec;
}

vector<double> WrapVectorActionTanh(EvalData &eval) {
    auto mat = eval.program_out->private_memory_[MemoryEigen::kVectorType_]
                   ->working_memory_[1];
    vector<double> vec(mat.data(), mat.data() + mat.rows() * mat.cols());
    for (auto &v : vec) v = std::tanh(v);  // TODO(skelly): better/faster way?
    return vec;
}

#endif