#include "point.h"

///****************************************************************************/
// string point::checkpoint(){
//	ostringstream oss;
//
//	oss <<"point:" << _id << ":" << _gtime << ":"  << _phase;
//	oss  << "|" << _pointState.size();
//	for (size_t i = 0; i <_pointState.size(); i++)
//		oss << ":" << _pointState[i];
//	oss  << "|" << _behaviouralState.size();
//	for (size_t i = 0; i <_behaviouralState.size(); i++)
//		oss << ":" << _behaviouralState[i];
//	oss << "|" << _auxDoubles.size();
//	for (size_t i = 0; i <_auxDoubles.size(); i++)
//		oss << ":" << _auxDoubles[i];
//	oss << endl;
//	return oss.str();
// }

/******************************************************************************/
bool point::isPointUnique(set<point *> &P) {
  vector<behaviourType> behaviour;
  set<point *>::iterator poiter;

  for (poiter = P.begin(); poiter != P.end(); poiter++) {
    (*poiter)->getBehaviour(behaviour);
    return !isEqual(_behaviour, behaviour, EPSILON_SBB);
  }
  return true;
}

///****************************************************************************/
// point::point(long gtime, long id, vector<behaviourType> &behaviour,
// vector<double> &auxDoubles, vector<int> &auxInts){
//    _gtime = gtime;
//    _id = id;
//    _behaviour = behaviour;
//    _auxDoubles = auxDoubles;
//    _auxInts = auxInts;
//    _key = 0;
// }

/******************************************************************************/
point::point(long gtime, long id, string behaviour, vector<double> &auxDoubles,
             vector<int> &auxInts) {
  _gtime = gtime;
  _id = id;
  _behaviourString = behaviour;
  _auxDoubles = auxDoubles;
  _auxInts = auxInts;
  _key = 0;
}

/******************************************************************************/
// partial copy constructor for parallel evaluations
point::point(point &p) {
  _gtime = p.gtime();
  _id = p.id();
  _key = 0;

  p.getBehaviour(_behaviour);
  p.auxDoubles(_auxDoubles);
  p.auxInts(_auxInts);
}

/******************************************************************************/
ostream &operator<<(ostream &os, const point &pt) {
  os << "(" << pt._id << ", " << pt._gtime << ")";
  return os;
}
