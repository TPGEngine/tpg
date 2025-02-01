#ifndef TIMING_METRICS_BUILDER_H
#define TIMING_METRICS_BUILDER_H

#include <timing_metrics.h>

class TimingMetricsBuilder {
public:

    TimingMetricsBuilder& with_generation(int generation);
    TimingMetricsBuilder& with_total_generation_time(double total_generation_time);
    TimingMetricsBuilder& with_evaluation_time(double evaluation_time);
    TimingMetricsBuilder& with_generate_teams_time(double generate_teams_time);
    TimingMetricsBuilder& with_set_elite_teams_time(double set_elite_teams_time);
    TimingMetricsBuilder& with_select_teams_time(double select_teams_time);
    TimingMetricsBuilder& with_report_time(double report_time);
    TimingMetricsBuilder& with_modes_time(double modes_time);
    TimingMetricsBuilder& with_lost_time(double lost_time);

    int get_generation() const;
    double get_total_generation_time() const;
    double get_evaluation_time() const;
    double get_generate_teams_time() const;
    double get_set_elite_teams_time() const;
    double get_select_teams_time() const;
    double get_report_time() const;
    double get_modes_time() const;
    double get_lost_time() const;

    TimingMetrics build() const;

private:
    int generation = 0;
    double total_generation_time = 0.0;
    double evaluation_time = 0.0;
    double generate_teams_time = 0.0;
    double set_elite_teams_time = 0.0;
    double select_teams_time = 0.0;
    double report_time = 0.0;
    double modes_time = 0.0;
    double lost_time = 0.0;
};


#endif