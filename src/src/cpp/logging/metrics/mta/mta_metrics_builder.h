#ifndef MTA_METRICS_BUILDER_H
#define MTA_METRICS_BUILDER_H

#include "mta_metrics.h"

class MTAMetricsBuilder {
public:
    MTAMetricsBuilder& with_generation(long generation);
    MTAMetricsBuilder& with_best_fitness(double best_fitness);
    MTAMetricsBuilder& with_team_id(long team_id);    
    MTAMetricsBuilder& with_team_size(int team_size);
    MTAMetricsBuilder& with_age(int age);
    MTAMetricsBuilder& with_fitness_value_for_selection(double fitness_value_for_selection);
    MTAMetricsBuilder& with_total_program_instructions(int total_program_instructions);
    MTAMetricsBuilder& with_total_effective_program_instructions(int total_effective_program_instructions);

    long get_generation() const;
    double get_best_fitness() const;
    long get_team_id() const;
    int get_team_size() const;
    long get_age() const;
    double get_fitness_value_for_selection() const;
    int get_total_program_instructions() const;
    int get_total_effective_program_instructions() const;

    MTAMetrics build() const;

private:
    long team_id = 0;
    long generation = 0;
    double best_fitness = 0.0;
    int team_size = 0;
    long age = 0;
    double fitness_value_for_selection = 0.0;
    int program_instruction_count = 0;
    int effective_program_instruction_count = 0;
};


#endif