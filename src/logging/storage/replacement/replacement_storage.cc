#include "replacement_storage.h"
#include <iomanip>

void ReplacementStorage::init(const int& seed_tpg, const int& pid) {
    std::stringstream filename;
    filename << "replacement." << seed_tpg << "." << pid << ".csv";

    file_.open(filename.str());
    file_ << "generation,num_teams,num_programs,memory_size,num_elite_teams,num_new_teams\n";
    file_.flush();
}

void ReplacementStorage::append(const ReplacementMetrics& metrics) {
    file_ << metrics.generation << ","
          << metrics.num_teams << ","
          << metrics.num_programs << ","
          << metrics.memory_size << ","
          << metrics.num_elite_teams << ","
          << metrics.num_new_teams << "\n";
    file_.flush();
}
