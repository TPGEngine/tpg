#include "selection_storage.h"
#include <iomanip>

void SelectionStorage::init(const int& seed_tpg, const int& pid) {
    std::stringstream filename;
    filename << "selection." << seed_tpg << "." << pid << ".csv";

    file_.open(filename.str());
    file_ << "generation,best_fitness,team_id,team_size,age,fitness_value_for_selection,program_instruction_count,effective_program_instruction_count\n";
    file_.flush();
}

void SelectionStorage::append(const SelectionMetrics& metrics) {
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
