#include "csv_storage.h"

CSVStorage& CSVStorage::instance() {
    static CSVStorage instance;
    return instance;
}

void CSVStorage::init(const std::string& filename) {
    file_.open(filename);
    file_ << "generation,best_fitness,team_id\n";
    file_.flush();
}

void CSVStorage::append(const std::map<std::string, std::string>& data) {
    file_ << data.at("generation") << ","
          << data.at("best_fitness") << ","
          << data.at("team_id") << "\n";
    file_.flush();
}
