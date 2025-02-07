#include "selection_metrics_builder.h"
#include "selection_metrics.h"
#include <string>
#include <sstream>

SelectionMetricsBuilder& SelectionMetricsBuilder::with_generation(long generation) {
    this->generation = generation;
    return *this;
}

SelectionMetricsBuilder& SelectionMetricsBuilder::with_best_fitness(double best_fitness) {
    this->best_fitness = best_fitness;
    return *this;
}

SelectionMetricsBuilder& SelectionMetricsBuilder::with_team_id(long team_id) {
    this->team_id = team_id;
    return *this;
}

SelectionMetricsBuilder& SelectionMetricsBuilder::with_team_size(int team_size) {
    this->team_size = team_size;
    return *this;
}

SelectionMetricsBuilder& SelectionMetricsBuilder::with_age(int age) {
    this->age = age;
    return *this;
}

SelectionMetricsBuilder& SelectionMetricsBuilder::with_fitness_value_for_selection(double fitness_value_for_selection) {
    this->fitness_value_for_selection = fitness_value_for_selection;
    return *this;
}

SelectionMetricsBuilder& SelectionMetricsBuilder::with_total_program_instructions(int total_program_instructions) {
    this->program_instruction_count = total_program_instructions;
    return *this;
}

SelectionMetricsBuilder& SelectionMetricsBuilder::with_total_effective_program_instructions(int total_effective_program_instructions) {
    this->effective_program_instruction_count = total_effective_program_instructions;
    return *this;
}

SelectionMetricsBuilder& SelectionMetricsBuilder::with_operations(std::vector<int> operations) {
    std::stringstream ss;

    bool first = true;
    for (int op : operations) {
        if (!first) {
            ss << ",";
        }
        ss << op;
        first = false;
    }

    this->operations = ss.str();
    return *this;
}

SelectionMetrics SelectionMetricsBuilder::build() const {
    return { *this };
}

long SelectionMetricsBuilder::get_generation() const {
    return generation;
}

double SelectionMetricsBuilder::get_best_fitness() const {
    return best_fitness;
}

long SelectionMetricsBuilder::get_team_id() const {
    return team_id;
}

int SelectionMetricsBuilder::get_team_size() const {
    return team_size;
}

long SelectionMetricsBuilder::get_age() const {
    return age;
}

double SelectionMetricsBuilder::get_fitness_value_for_selection() const {
    return fitness_value_for_selection;
}

int SelectionMetricsBuilder::get_total_program_instructions() const {
    return program_instruction_count;
}

int SelectionMetricsBuilder::get_total_effective_program_instructions() const {
    return effective_program_instruction_count;
}

std::string SelectionMetricsBuilder::get_operations() const {
    return operations;
}
