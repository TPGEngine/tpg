#include "removal_metrics_builder.h"

RemovalMetricsBuilder& RemovalMetricsBuilder::with_generation(
    long generation) {
  generation_ = generation;
  return *this;
}

RemovalMetricsBuilder& RemovalMetricsBuilder::with_num_teams(
    std::size_t num_teams) {
  num_teams_ = num_teams;
  return *this;
}

RemovalMetricsBuilder& RemovalMetricsBuilder::with_num_programs(
    std::size_t num_programs) {
  num_programs_ = num_programs;
  return *this;
}

RemovalMetricsBuilder& RemovalMetricsBuilder::with_num_root_programs(
    int num_root_programs) {
  num_root_programs_ = num_root_programs;
  return *this;
}

RemovalMetricsBuilder& RemovalMetricsBuilder::with_num_elite_teams(
    uint_fast32_t num_elite_teams) {
  num_elite_teams_ = num_elite_teams;
  return *this;
}

RemovalMetricsBuilder& RemovalMetricsBuilder::with_num_deleted(int num_deleted) {
  num_deleted_ = num_deleted;
  return *this;
}

RemovalMetricsBuilder& RemovalMetricsBuilder::with_num_old_deleted(
    int num_old_deleted) {
  num_old_deleted_ = num_old_deleted;
  return *this;
}

RemovalMetricsBuilder& RemovalMetricsBuilder::with_percent_old_deleted(
    double percent_old_deleted) {
  percent_old_deleted_ = percent_old_deleted;
  return *this;
}


RemovalMetrics RemovalMetricsBuilder::build() const {
  return RemovalMetrics(generation_, num_teams_, num_programs_, num_root_programs_,
                        num_elite_teams_,num_deleted_, num_old_deleted_, percent_old_deleted_);
}