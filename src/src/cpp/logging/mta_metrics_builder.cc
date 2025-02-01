#include <mta_metrics_builder.h>
#include <mta_metrics.h>

MTAMetricsBuilder& MTAMetricsBuilder::with_generation(long generation) {
    this->generation = generation;
    return *this;
}

MTAMetricsBuilder& MTAMetricsBuilder::with_best_fitness(double best_fitness) {
    this->best_fitness = best_fitness;
    return *this;
}

MTAMetricsBuilder& MTAMetricsBuilder::with_team_id(long team_id) {
    this->team_id = team_id;
    return *this;
}

MTAMetricsBuilder& MTAMetricsBuilder::with_team_size(int team_size) {
    this->team_size = team_size;
    return *this;
}

MTAMetricsBuilder& MTAMetricsBuilder::with_age(int age) {
    this->age = age;
    return *this;
}

MTAMetricsBuilder& MTAMetricsBuilder::with_fitness_value_for_selection(double fitness_value_for_selection) {
    this->fitness_value_for_selection = fitness_value_for_selection;
    return *this;
}

MTAMetricsBuilder& MTAMetricsBuilder::with_total_program_instructions(int total_program_instructions) {
    this->program_instruction_count = total_program_instructions;
    return *this;
}

MTAMetricsBuilder& MTAMetricsBuilder::with_total_effective_program_instructions(int total_effective_program_instructions) {
    this->effective_program_instruction_count = total_effective_program_instructions;
    return *this;
}

MTAMetrics MTAMetricsBuilder::build() const {
    return MTAMetrics(*this);
}

long MTAMetricsBuilder::get_generation() const {
    return generation;
}

double MTAMetricsBuilder::get_best_fitness() const {
    return best_fitness;
}

long MTAMetricsBuilder::get_team_id() const {
    return team_id;
}

int MTAMetricsBuilder::get_team_size() const {
    return team_size;
}

long MTAMetricsBuilder::get_age() const {
    return age;
}

double MTAMetricsBuilder::get_fitness_value_for_selection() const {
    return fitness_value_for_selection;
}

int MTAMetricsBuilder::get_total_program_instructions() const {
    return program_instruction_count;
}

int MTAMetricsBuilder::get_total_effective_program_instructions() const {
    return effective_program_instruction_count;
}