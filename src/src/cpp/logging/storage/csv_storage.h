#pragma once
#include <fstream>
#include <map>
#include <mutex>
#include <sstream>

template<typename T>
class CSVStorage {
public:
    virtual ~CSVStorage() = default;

    virtual void init(const int& seed_tpg, const int& pid) = 0;
    virtual void append(const T& metrics) = 0;

    // Disable copy and move operations.
    CSVStorage(const CSVStorage&) = delete;
    CSVStorage& operator=(const CSVStorage&) = delete;
    CSVStorage(CSVStorage&&) = delete;
    CSVStorage& operator=(CSVStorage&&) = delete;

protected:
    CSVStorage() = default;
    std::ofstream file_;
};
