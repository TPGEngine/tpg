#ifndef point_h
#define point_h

#include <map>
#include <set>
#include <vector>

#include "misc.h"

#define POINT_AUX_INT_TASK 0
#define POINT_AUX_INT_PHASE 1
#define POINT_AUX_INT_ENVSEED 2
#define POINT_AUX_INT_internalTestNodeId 3

using namespace std;

class point {
 public:
  inline double auxDouble(int i) { return _auxDoubles[i]; }
  inline void auxDouble(int i, double d) { _auxDoubles[i] = d; }
  inline double auxInt(int i) { return _auxInts[i]; }
  inline void auxInt(int i, int j) { _auxInts[i] = j; }
  inline void auxDoubles(vector<double> &v) const { v = _auxDoubles; }
  inline void auxInts(vector<int> &v) const { v = _auxInts; }
  inline string desc() { return _desc; }
  inline void desc(string d) { _desc = d; }
  inline int task() const { return _auxInts[POINT_AUX_INT_TASK]; }
  inline int phase() const { return _auxInts[POINT_AUX_INT_PHASE]; }
  inline void phase(int p) { _auxInts[POINT_AUX_INT_PHASE] = p; }
  inline int envSeed() const { return _auxInts[POINT_AUX_INT_ENVSEED]; }
  inline void getBehaviour(vector<behaviourType> &v) const { v = _behaviour; }
  inline void getBehaviourString(string &s) const { s = _behaviourString; }
  inline string getBehaviourString() const { return _behaviourString; }
  inline long gtime() { return _gtime; }
  inline long id() { return _id; }
  inline void id(long i) { _id = i; }
  inline double key() { return _key; }
  inline void key(double key) { _key = key; }
  void setBehaviour(vector<behaviourType> &v) { _behaviour = v; }

  point(long, long, string, vector<double> &, vector<int> &);

  point(point &p);

  // Return true if the point is unique w.r.t. another point
  bool isPointUnique(set<point *> &);
  friend ostream &operator<<(ostream &, const point &);

  // protected:

  vector<double> _auxDoubles;
  vector<int> _auxInts;  // task, phase, envSeed, internalTestNodeId
  vector<behaviourType> _behaviour;
  string _behaviourString;
  long _gtime;
  long _id;
  double _key; /* For sorting. */
  string _desc;
};

struct pointLexicalLessThan {
  bool operator()(point *p1, point *p2) const {
    // if (!isEqual(p1->key(), p2->key()))
    //    return p1->key() > p2->key();
    return p1->id() < p2->id();
  }
};

#endif
