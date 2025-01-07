#ifndef state_h
#define state_h

#include <set>
#include <vector>

#include "misc.h"

class state {
 public:
  long dim_;
  size_t scalarStateType;  // 0: double, 1: short, 2:float

  inline double getStateVarDouble(int v) { return (*featVecDouble_)[v]; }

  inline vector<double>* getStatePointerDouble() { return featVecDouble_; }

  inline double stateValueAtIndex(size_t i) {
    if (scalarStateType == 0)
      return (*featVecDouble_)[i];
    else if (scalarStateType == 1)
      return (double)(*featVecShort_)[i];
    else
      return (double)(*featVecFloat_)[i];
  }

  inline void Set(vector<double>& s) { featVecDouble_ = &s; }

  inline void Set(vector<float>& s) { featVecFloat_ = &s; }

  inline void Set(vector<short>& s) { featVecShort_ = &s; }
  state() {}
  state(long d) {
    dim_ = d;
    scalarStateType = 0;
  }
  ~state() {}

  vector<float>* featVecFloat_;
  vector<double>* featVecDouble_;
  vector<short>* featVecShort_;
};
#endif
