#include "selection_storage.h"
#include "instruction.h"
#include <iomanip>

void SelectionStorage::init(const int& seed_tpg, const int& pid) {
    std::string filename = generate_filename("selection", seed_tpg, pid);

    file_.open(filename);
    file_ << "generation,best_fitness,team_id,team_size,age,fitness_value_for_selection,program_instruction_count,effective_program_instruction_count";
    
    appendOperationHeaders();

    file_ << "\n";
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
          << metrics.effective_program_instruction_count << ","
          << metrics.operations_use << "\n";
    file_.flush();
}

void SelectionStorage::appendOperationHeaders() {
    for (std::string op_name : instruction::op_names_) {
        std::transform(op_name.begin(), op_name.end(), op_name.begin(), ::tolower);
        file_ << "," << op_name;
    }
}