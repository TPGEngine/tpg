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

protected:
    CSVStorage() = default;
    std::ofstream file_;
};
