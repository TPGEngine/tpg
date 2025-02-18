#include "removal_storage.h"
#include <iomanip>

void RemovalStorage::init(const int& seed_tpg, const int& pid) {
    std::stringstream filename;
    filename << "removal." << seed_tpg << "." << pid << ".csv";

    file_.open(filename.str());
    file_ << "generation,num_teams,num_programs,num_root_programs,num_elite_teams,num_deleted,num_old_deleted,percent_old_deleted\n";
    file_.flush();
}

void RemovalStorage::append(const RemovalMetrics& metrics) {
    file_ << metrics.generation << ","
          << metrics.num_teams << ","
          << metrics.num_programs << ","
          << metrics.num_root_programs << ","
          << metrics.num_elite_teams << ","
          << metrics.num_deleted << ","
          << metrics.num_old_deleted << ","
          << std::fixed << std::setprecision(6) << metrics.percent_old_deleted << "\n";
    file_.flush();
}
