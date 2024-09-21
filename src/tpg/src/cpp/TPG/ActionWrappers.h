#ifndef ActionWrappers_h
#define ActionWrappers_h

#include "EvalData.h"

// TPG represents discrete actions as negative ints starting at -1
// Map them to positive ints starting at 0
int WrapDiscreteAction(EvalData &eval) {
    return (eval.program_out->action() * -1) - 1;
}

double WrapContinuousAction(EvalData &eval) {
    return eval.program_out->privateMemory_[memoryEigen::SCALAR_TYPE]
        ->working_memory_[1](0, 0);
}

double WrapContinuousActionSigmoid(EvalData &eval) {
    double p = eval.program_out->privateMemory_[memoryEigen::SCALAR_TYPE]
                   ->working_memory_[1](0, 0);
    return 1 / (1 + exp(-p));
}

vector<double> WrapVectorActionSigmoid(EvalData &eval) {
    auto mat = eval.program_out->privateMemory_[memoryEigen::VECTOR_TYPE]
                   ->working_memory_[1];
    vector<double> vec(mat.data(), mat.data() + mat.rows() * mat.cols());
    for (auto &v : vec) v = sigmoid(v);  // TODO(skelly): better/faster way?
    return vec;
}

#endif