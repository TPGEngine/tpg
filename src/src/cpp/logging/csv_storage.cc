#include "csv_storage.h"
#include <boost/mpi.hpp>

CSVStorage& CSVStorage::instance() {
    static CSVStorage instance;
    return instance;
}

void CSVStorage::init(const std::string& filename, const boost::mpi::communicator& world) {
    if (world.rank() == 0) {  // Only master rank opens file
        file_.open(filename);
        file_ << "generation,best_fitness,team_id\n";
        file_.flush();
    }
}

void CSVStorage::append(const std::map<std::string, std::string>& data, const boost::mpi::communicator& world) {
    if (world.rank() != 0) return;  // Only master writes

    std::lock_guard<std::mutex> lock(mutex_);
    file_ << data.at("generation") << ","
          << data.at("best_fitness") << ","
          << data.at("team_id") << "\n";
    file_.flush();
}
