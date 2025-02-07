#ifndef SELECTION_METRICS_H
#define SELECTION_METRICS_H

#include <vector>
#include <optional>
#include <string>

class SelectionMetricsBuilder;

struct SelectionMetrics {
    const long team_id;
    const long generation;
    const double best_fitness;
    const int team_size;
    const long age;
    const double fitness_value_for_selection;
    const int program_instruction_count;
    const int effective_program_instruction_count;
    const std::string operations;

    SelectionMetrics(const SelectionMetricsBuilder& builder);
};

#include "selection_metrics_builder.h"

#endif