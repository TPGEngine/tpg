#pragma once
#include <fstream>
#include <map>
#include <mutex>
#include <sstream>

class CSVStorage {
public:
    virtual ~CSVStorage() = default;

    virtual void init(const int& seed_tpg, const int& pid) = 0;
    virtual void append(const std::map<std::string, std::string>& data) = 0;

protected:
    CSVStorage() = default;
    std::ofstream file_;
};
