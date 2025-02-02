#include "mta_storage.h"
#include <iomanip>

void MTAStorage::init(const int& seed_tpg, const int& pid) {
    std::stringstream filename;
    filename << "mta." << seed_tpg << "." << pid << ".csv";

    file_.open(filename.str());
    file_ << "generation,best_fitness,team_id,team_size,age,fitness_value_for_selection,program_instruction_count,effective_program_instruction_count\n";
    file_.flush();
}

void MTAStorage::append(const MTAMetrics& metrics) {
    file_ << metrics.generation << ","
          << std::fixed << std::setprecision(6) << metrics.best_fitness << ","
          << metrics.team_id << ","
          << metrics.team_size << ","
          << metrics.age << ","
          << metrics.fitness_value_for_selection << ","
          << metrics.program_instruction_count << ","
          << metrics.effective_program_instruction_count << "\n";
    file_.flush();
}
