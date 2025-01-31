#include "timing_storage.h"

void TimingStorage::init(const int& seed_tpg, const int& pid) {
    std::stringstream filename;
    filename << "tms." << seed_tpg << "." << pid << ".csv";

    file_.open(filename.str());
    file_ << "type,generation,best_fitness,team_id\n";
    file_.flush();
}

void TimingStorage::append(const std::map<std::string, std::string>& data) {
    file_ << data.at("type") << ","
          << data.at("generation") << ","
          << data.at("best_fitness") << ","
          << data.at("team_id") << "\n";
    file_.flush();
}

