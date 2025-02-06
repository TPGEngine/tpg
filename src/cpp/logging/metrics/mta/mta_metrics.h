#ifndef MTA_METRICS_H
#define MTA_METRICS_H

#include <vector>
#include <optional>

class MTAMetricsBuilder;

struct MTAMetrics {
    const long team_id;
    const long generation;
    const double best_fitness;
    const int team_size;
    const long age;
    const double fitness_value_for_selection;
    const int program_instruction_count;
    const int effective_program_instruction_count;

    MTAMetrics(const MTAMetricsBuilder& builder);
};

#include "mta_metrics_builder.h"

#endif