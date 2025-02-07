#ifndef SELECTION_METRICS_BUILDER_H
#define SELECTION_METRICS_BUILDER_H

#include "selection_metrics.h"
#include <string>

class SelectionMetricsBuilder {
public:
    SelectionMetricsBuilder& with_generation(long generation);
    SelectionMetricsBuilder& with_best_fitness(double best_fitness);
    SelectionMetricsBuilder& with_team_id(long team_id);    
    SelectionMetricsBuilder& with_team_size(int team_size);
    SelectionMetricsBuilder& with_age(int age);
    SelectionMetricsBuilder& with_fitness_value_for_selection(double fitness_value_for_selection);
    SelectionMetricsBuilder& with_total_program_instructions(int total_program_instructions);
    SelectionMetricsBuilder& with_total_effective_program_instructions(int total_effective_program_instructions);
    SelectionMetricsBuilder& with_operations(std::vector<int> operations);

    long get_generation() const;
    double get_best_fitness() const;
    long get_team_id() const;
    int get_team_size() const;
    long get_age() const;
    double get_fitness_value_for_selection() const;
    int get_total_program_instructions() const;
    int get_total_effective_program_instructions() const;
    std::string get_operations() const;

    SelectionMetrics build() const;

private:
    long team_id = 0;
    long generation = 0;
    double best_fitness = 0.0;
    int team_size = 0;
    long age = 0;
    double fitness_value_for_selection = 0.0;
    int program_instruction_count = 0;
    int effective_program_instruction_count = 0;
    std::string operations = "";
};


#endif