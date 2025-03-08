#pragma once
#include <filesystem>
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

    void ensure_directory_exists(const std::string& dir) {
        if (!std::filesystem::exists(dir)) {
            std::filesystem::create_directories(dir);
        }
    }

    std::string generate_filename(const std::string& base_name, int seed_tpg, int pid) {
        std::stringstream filename;
        std::string dir = "logs/";
        ensure_directory_exists(dir);
        filename << dir << base_name << "." << seed_tpg << "." << pid << ".csv";
        return filename.str();
    }
};
