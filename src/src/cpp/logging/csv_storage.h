#pragma once
#include <fstream>
#include <map>
#include <mutex>

class CSVStorage {
public:
    static CSVStorage& instance();
    
    void init(const std::string& filename);
    void append(const std::map<std::string, std::string>& data);

private:
    std::ofstream file_;
};
