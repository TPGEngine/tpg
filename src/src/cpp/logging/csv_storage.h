#pragma once
#include <boost/mpi.hpp>
#include <fstream>
#include <map>
#include <mutex>

class CSVStorage {
public:
    static CSVStorage& instance();
    
    void init(const std::string& filename, const boost::mpi::communicator& world);
    void append(const std::map<std::string, std::string>& data, const boost::mpi::communicator& world);

private:
    std::ofstream file_;
    std::mutex mutex_;
};
