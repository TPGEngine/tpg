#include "selection_metrics.h"
#include "selection_metrics_builder.h"

SelectionMetrics::SelectionMetrics(const SelectionMetricsBuilder& builder)
    : team_id(builder.get_team_id()),
      generation(builder.get_generation()),
      best_fitness(builder.get_best_fitness()),
      team_size(builder.get_team_size()),
      age(builder.get_age()),
      fitness_value_for_selection(builder.get_fitness_value_for_selection()),
      program_instruction_count(builder.get_total_program_instructions()),
      effective_program_instruction_count(builder.get_total_effective_program_instructions()),
      operations_use(builder.get_operations_use()) {
}