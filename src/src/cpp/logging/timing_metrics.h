#ifndef TIMING_METRICS_H
#define TIMING_METRICS_H

#include <vector>
#include <optional>
#include "timing_metrics_builder.h"


struct TimingMetrics {
    const int generation;
    const double total_generation_time;
    const double evaluation_time;
    const double generate_teams_time;
    const double set_elite_teams_time;
    const double select_teams_time;
    const double report_time;
    const double modes_time;
    const double lost_time;

    TimingMetrics(const TimingMetricsBuilder& builder);

};

#endif