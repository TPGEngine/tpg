#include "csv_storage.h"
#include <sstream>

CSVStorage& CSVStorage::instance() {
    static CSVStorage instance;
    return instance;
}

void CSVStorage::init(const int& seed_tpg, const int& pid) {
    std::stringstream filename;
    filename << "tpg." << seed_tpg << "." << pid << ".csv";

    file_.open(filename.str());
    file_ << "generation,best_fitness,team_id\n";
    file_.flush();
}

void CSVStorage::append(const std::map<std::string, std::string>& data) {
    file_ << data.at("generation") << ","
          << data.at("best_fitness") << ","
          << data.at("team_id") << "\n";
    file_.flush();
}
