#ifndef memoryEigen_h
#define memoryEigen_h

#include <Eigen/Dense>
#include <any>
#include <random>

#include "misc.h"

using namespace Eigen;

typedef Matrix<bool, Dynamic, Dynamic> MatrixXb;
class memoryEigen {
   public:
    static const size_t SCALAR_TYPE = 0;
    static const size_t VECTOR_TYPE = 1;
    static const size_t MATRIX_TYPE = 2;
    static const size_t NA_TYPE = 3;
    static const int NUM_MEMORY_TYPES = 3;

    inline Ref<MatrixXb> getActiveE() { return active_; }
    inline Ref<MatrixXd> getReadTimeE() { return read_time_; }
    inline Ref<MatrixXd> getWriteTimeE() { return write_time_; }

    string checkpoint() {
        ostringstream oss;
        const static IOFormat CPFormat(StreamPrecision, DontAlignCols, ":",
                                       ":");
        oss << "memoryEigen:" << id_ << ":" << type_ << ":" << memoryIndices_
            << ":" << memory_size_ << ":" << nrefs_;
        for (auto &m : const_memory_) {
            oss << ":" << m.format(CPFormat);
        }
        oss << endl;
        return oss.str();
    }
    inline string PrintWorking() {
        ostringstream oss;
        for (auto &m : working_memory_) {
            oss << ":" << m;
        }
        oss << endl;
        return oss.str();
    }
    inline void ClearWorking() {
        for (auto &m : working_memory_) {
            m.setZero();
        }
    }
    inline void ClearConst() {
        for (auto &m : const_memory_) {
            m.setZero();
        }
    }
    inline void RandomizeConst() {
        for (auto &m : const_memory_) {
            m.setRandom();
        }
    }
    inline void NoiseToConst(mt19937 &rng, double stddev) {
        auto dis = std::normal_distribution<double>(0, stddev);
        for (size_t i = 0; i < const_memory_.size(); i++) {
            for (auto &x : const_memory_[i].reshaped()) x += dis(rng);
        }
    }
    inline void CopyConstToWorking() {
        for (size_t i = 0; i < const_memory_.size(); i++) {
            working_memory_[i] = const_memory_[i];
        }
    }
    inline void ClearActive() { active_.fill(false); }
    inline void SetActive() { active_.fill(true); }
    inline void ClearReadTime() { read_time_.setZero(); }
    inline void ClearWriteTime() { write_time_.setZero(); }
    inline long id() { return id_; }
    inline void id(long id) { id_ = id; }
    constexpr size_t indexSize() { return memoryIndices_; }
    inline void getActiveReadTime(std::vector<double> &v) {
        for (size_t i = 0; i < memoryIndices_; i++)
            if (active_(i, 0)) v[i] = read_time_(i, 0);
    }
    inline void getActiveWriteTime(std::vector<double> &v) {
        for (size_t i = 0; i < memoryIndices_; i++)
            if (active_(i, 0)) v[i] = write_time_(i, 0);
    }
    inline int refs() { return nrefs_; }
    inline void refs(int i) { nrefs_ = i; }
    inline int refDec() { return --nrefs_; }
    inline int refInc() { return ++nrefs_; }
    inline int RefsPolicy() { return nrefs_policy_; }
    inline void RefsPolicy(int i) { nrefs_policy_ = i; }
    inline int RefsPolicyInc() { return ++nrefs_policy_; }
    void ResizeMemory() {
        working_memory_.resize(memoryIndices_);
        const_memory_.resize(memoryIndices_);
        for (size_t i = 0; i < memoryIndices_; i++) {
            if (type_ == SCALAR_TYPE) {
                working_memory_[i].resize(1, 1);
                const_memory_[i].resize(1, 1);
            } else if (type_ == VECTOR_TYPE) {
                working_memory_[i].resize(memory_size_, 1);
                const_memory_[i].resize(memory_size_, 1);
            } else if (type_ == MATRIX_TYPE) {
                working_memory_[i].resize(memory_size_, memory_size_);
                const_memory_[i].resize(memory_size_, memory_size_);
            }
        }
        active_.resize(memoryIndices_, 1);
        read_time_.resize(memoryIndices_, 1);
        write_time_.resize(memoryIndices_, 1);
    }
    memoryEigen(long i, int type, size_t memoryIndices, size_t memory_size) {
        id_ = i;
        nrefs_ = 0;
        type_ = type;
        memoryIndices_ = memoryIndices;
        memory_size_ = memory_size;
        ResizeMemory();
        ClearWorking();
        RandomizeConst();
        ClearActive();
        ClearReadTime();
        ClearWriteTime();
    }

    memoryEigen(std::vector<std::string> &outcomeFields) {
        size_t i = 1;
        id_ = atoi(outcomeFields[i++].c_str());
        type_ = atoi(outcomeFields[i++].c_str());
        memoryIndices_ = atoi(outcomeFields[i++].c_str());
        memory_size_ = atoi(outcomeFields[i++].c_str());
        nrefs_ = atoi(outcomeFields[i++].c_str());
        ResizeMemory();
        ClearWorking();
        // ClearConst();
        ClearActive();
        ClearReadTime();
        ClearWriteTime();
        // Read in evolved constants
        for (size_t idx = 0; idx < memoryIndices_; idx++) {
            if (type_ == memoryEigen::SCALAR_TYPE) {
                const_memory_[idx](0, 0) = stod(outcomeFields[i++].c_str());
            } else if (type_ == memoryEigen::VECTOR_TYPE) {
                for (size_t r = 0; r < memory_size_; r++)
                    const_memory_[idx](r, 0) = stod(outcomeFields[i++].c_str());
            } else if (type_ == memoryEigen::MATRIX_TYPE) {
                for (size_t r = 0; r < memory_size_; r++)
                    for (size_t c = 0; c < memory_size_; c++)
                        const_memory_[idx](r, c) =
                            stod(outcomeFields[i++].c_str());
            }
        }
    }

    ~memoryEigen() {}
    long id_;
    int type_;
    size_t memoryIndices_;
    size_t memory_size_;
    std::deque<Matrix<double, Dynamic, Dynamic> > working_memory_;
    std::vector<Matrix<double, Dynamic, Dynamic> > const_memory_;
    Matrix<bool, Dynamic, 1> active_;
    Matrix<double, Dynamic, 1> read_time_;
    Matrix<double, Dynamic, 1> write_time_;
    int nrefs_;         // Num references by programs
    int nrefs_policy_;  // Num references within a particular policy graph
};

struct memoryEigenIdComp {
    bool operator()(memoryEigen *m1, memoryEigen *m2) const {
        return m1->id() < m2->id();
    }
};

#endif
