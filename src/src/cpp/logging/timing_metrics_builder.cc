#include "timing_metrics_builder.h"
#include "timing_metrics.h"

TimingMetricsBuilder& TimingMetricsBuilder::with_generation(int generation) {
    this->generation = generation;
    return *this;
}

TimingMetricsBuilder& TimingMetricsBuilder::with_total_generation_time(double total_generation_time) {
    this->total_generation_time = total_generation_time;
    return *this;
}

TimingMetricsBuilder& TimingMetricsBuilder::with_evaluation_time(double evaluation_time) {
    this->evaluation_time = evaluation_time;
    return *this;
}

TimingMetricsBuilder& TimingMetricsBuilder::with_generate_teams_time(double generate_teams_time) {
    this->generate_teams_time = generate_teams_time;
    return *this;
}

TimingMetricsBuilder& TimingMetricsBuilder::with_set_elite_teams_time(double set_elite_teams_time) {
    this->set_elite_teams_time = set_elite_teams_time;
    return *this;
}

TimingMetricsBuilder& TimingMetricsBuilder::with_select_teams_time(double select_teams_time) {
    this->select_teams_time = select_teams_time;
    return *this;
}

TimingMetricsBuilder& TimingMetricsBuilder::with_report_time(double report_time) {
    this->report_time = report_time;
    return *this;
}

TimingMetricsBuilder& TimingMetricsBuilder::with_modes_time(double modes_time) {
    this->modes_time = modes_time;
    return *this;
}

TimingMetricsBuilder& TimingMetricsBuilder::with_lost_time(double lost_time) {
    this->lost_time = lost_time;
    return *this;
}

int TimingMetricsBuilder::get_generation() const {
    return generation;
}

double TimingMetricsBuilder::get_total_generation_time() const {
    return total_generation_time;
}

double TimingMetricsBuilder::get_evaluation_time() const {
    return evaluation_time;
}

double TimingMetricsBuilder::get_generate_teams_time() const {
    return generate_teams_time;
}

double TimingMetricsBuilder::get_set_elite_teams_time() const {
    return set_elite_teams_time;
}

double TimingMetricsBuilder::get_select_teams_time() const {
    return select_teams_time;
}

double TimingMetricsBuilder::get_report_time() const {
    return report_time;
}

double TimingMetricsBuilder::get_modes_time() const {
    return modes_time;
}

double TimingMetricsBuilder::get_lost_time() const {
    return lost_time;
}

TimingMetrics TimingMetricsBuilder::build() const {
    return TimingMetrics(*this);
}

