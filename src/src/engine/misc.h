#ifndef misc_h
#define misc_h

#include <algorithm>
#include <any>
#include <chrono>
#include <cmath>
#include <cstring>
#include <deque>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <random>
#include <set>
#include <sstream>
#include <unordered_map>
#include <vector>

using namespace std;
using std::numeric_limits;
typedef double behaviourType;

#define NEARZERO 10e-12
#define EPSILON_SBB 1e-5
#define MAX_NCD 1.2
#define _MEAN_OUT_PROP 1.0
#define MIN_NIBBLE_VAL 0
#define MAX_NIBBLE_VAL 15
#define _TRAIN_PHASE 0
#define _VALIDATION_PHASE 1
#define _TEST_PHASE 2
#define _PLAY_PHASE 3
#define _NUM_PHASE 4

int compressedLength(char *);

inline double bound(double x, double m, double M) { return min(max(x, m), M); }
inline double discretize(double f, double min, double max, int steps) {
    double d = round(((f - min) / (max - min)) * (steps - 1));
    return d > steps ? steps - 1 : d;
}
void die(const char *, const char *, const int, const char *);
// double EuclideanDistSqrd(double *, double *, int);
// double EuclideanDistSqrd(std::vector<double> &, std::vector<double> &);
// double EuclideanDistSqrdNorm(std::vector<double> &, std::vector<double> &);
// double EuclideanDist(std::vector<double> &, std::vector<double> &);

inline bool fileExists(const char *fileName) {
    ifstream infile(fileName);
    return infile.good();
}

inline bool isEqual(double x, double y) { return fabs(x - y) < NEARZERO; }

inline bool isEqual(double x, double y, double e) { return fabs(x - y) < e; }

inline bool isEqual(unsigned char x, unsigned char y) {
    return fabs(x - y) < 0;
}

bool isEqual(std::vector<int> &, std::vector<int> &);
bool isEqual(std::vector<double> &, std::vector<double> &, double);
template <class T>
bool isEqual(std::vector<T> &, std::vector<T> &, double);
inline bool isGreater(double x, double y, double e, bool orEqual = false) {
    return orEqual ? x > y || fabs(x - y) < e : x > y && fabs(x - y) > e;
}
inline bool isLess(double x, double y, double e, bool orEqual = false) {
    return orEqual ? x < y || fabs(x - y) < e : x < y && fabs(x - y) > e;
}

struct modesRecord {
    set<long> activeProgramIds;
    set<long> activeTeamIds;
    string behaviourString;
    long effectiveInstructionsTotal;
    double runTimeComplexityIns;
    modesRecord() {
        effectiveInstructionsTotal = 0;
        runTimeComplexityIns = 0;
    }
};

struct noveltyDescriptor {
    double novelty;
    std::vector<int> profile;
    std::vector<long> profileLong;
};

struct phyloRecord {
    std::vector<long> adj;  // children
    set<long> ancestorIds;
    string behaviourString;
    long gtime;
    long dtime;
    string fitnessBin;
    double fitness;
    vector<double> taskFitnesses;
    bool root;
    long numActiveFeatures;
    long numActivePrograms;
    long numActiveTeams;
    long numEffectiveInstructions;
    phyloRecord() {
        gtime = -1;
        dtime = -1;
        fitnessBin = "";
        fitness = 0;
        root = true;
    }
};

inline void getAncestorIds(map<long, phyloRecord> &phyloGraph, set<long> &a,
                           long id) {
    a.insert(phyloGraph[id].ancestorIds.begin(),
             phyloGraph[id].ancestorIds.end());
    for (auto it = phyloGraph[id].ancestorIds.begin();
         it != phyloGraph[id].ancestorIds.end(); it++)
        getAncestorIds(phyloGraph, a, *it);
}

int readMap(string, map<string, string> &);
void ReadParameters(string file_name,
                    std::unordered_map<string, std::any> &params);
inline double sigmoid(double x, double m) { return 1 / (1 + exp(-(m * x))); }
inline double sigmoid(double x) { return 1 / (1 + exp(-x)); }
double stdDev(std::vector<double>);
int stringToInt(string);
long stringToLong(string);
double stringToDouble(string);

inline double sas(double s1, double s2, double a) {
    return sqrt(pow(s1, 2) + pow(s2, 2) -
                (2 * s1 * s2 * cos(a * (3.14159265 / 180.0))));
}
std::vector<string> &SplitString(const string &s, char delim,
                                 std::vector<string> &elems);
std::vector<string> SplitString(const string &s, char delim);

template <class vtype>
string VectorToString(std::vector<vtype> &v, string delim = " ") {
    ostringstream oss;  // oss.precision(numeric_limits<double>::digits10+1);
    for (size_t i = 0; i < v.size(); i++) {
        oss << v[i];
        if (i < v.size() - 1) oss << delim;
    }
    return oss.str();
}
template <class vtype>
string setToStr(set<vtype> &s) {
    ostringstream oss;  // oss.precision(numeric_limits<double>::digits10+1);
    for (auto it = s.begin(); it != s.end(); it++) {
        oss << *it;
        if (next(it) != s.end()) oss << " ";
    }
    return oss.str();
}
template <class vtype>
string VectorToStringNoSpace(std::vector<vtype> &v) {
    ostringstream oss;
    for (size_t i = 0; i < v.size(); i++) {
        oss << v[i];
    }
    return oss.str();
}

/******************************************************************************/
template <class vtype>
double VectorMedian(std::vector<vtype> vec) {
   auto n = vec.size();

   if (n == 0) {
      die(__FILE__, __FUNCTION__, __LINE__,
          "trying to get median of empty vector");
   }
   size_t median_idx = n / 2;

   double m = 0;
   if (n % 2 == 1) {
      std::nth_element(vec.begin(), vec.begin() + median_idx, vec.end());
      m = vec[median_idx];
   } else {
      std::nth_element(vec.begin(), vec.begin() + median_idx - 1, vec.end());
      double mid1 = vec[median_idx - 1];
      std::nth_element(vec.begin(), vec.begin() + median_idx, vec.end());
      double mid2 = vec[median_idx];
      m = (mid1 + mid2) / 2.0;
   }
//    std::sort(vec.begin(), vec.end());
//    cerr << "dbg " << VectorToString(vec)  << ":" << m << endl;
   return m;
}

/******************************************************************************/
template <class vtype>
double VectorMean(std::vector<vtype> vec) {
   auto n = vec.size();

   if (n == 0) {
      die(__FILE__, __FUNCTION__, __LINE__,
          "trying to get mean of empty vector");
   }
   return accumulate(vec.begin(), vec.end(), 0.0) / n;
}

// Function to generate power set PS of given set S
inline void FindPowerSet(std::vector<int> const &S, std::vector<int> &set,
                         std::vector<std::vector<int>> &PS, size_t n,
                         size_t minSubsetSize) {
    // if we have considered all elements
    if (n == 0) {
        if (set.size() >= minSubsetSize) PS.push_back(set);
        return;
    }
    // consider nth element
    set.push_back(S[n - 1]);
    FindPowerSet(S, set, PS, n - 1, minSubsetSize);
    // or don't consider nth element
    set.pop_back();
    FindPowerSet(S, set, PS, n - 1, minSubsetSize);
}

inline std::vector<std::vector<int>> PowerSet(size_t n) {
    std::vector<int> S(n);
    std::iota(S.begin(), S.end(), 0);
    std::vector<int> tmpSet;
    std::vector<std::vector<int>> PS;
    FindPowerSet(S, tmpSet, PS, n, 1);
    return PS;
}

/**
 * Takes a set S and splits it into two disjoint sets A and B, where A has size
 * n.
 */
template <typename T>
inline void SplitSet(std::vector<T> S, std::vector<T> &A, std::vector<T> &B,
                     int n, std::mt19937 &g) {
    std::shuffle(S.begin(), S.end(), g);

    A = std::vector<T>(S.begin(), S.begin() + n);
    B = std::vector<T>(S.begin() + n, S.end());
}

inline double RoundTo(double value, double precision = 1.0) {
    return std::round(value / precision) * precision;
}

std::string ExpandEnvVars(const std::string &str);

/******************************************************************************/
inline void WriteStringToFile(const std::string& file_name,
                               const std::string& str) {
   std::ofstream ofs(file_name, std::ios::out);
   if (!ofs.is_open()) {
      die(__FILE__, __FUNCTION__, __LINE__, "Problem opening file.");
   }
   ofs << str;
   if (!ofs) {
      die(__FILE__, __FUNCTION__, __LINE__, "Problem writing to file.");
   }
}

#endif
