// replacement_metrics_builder.cpp
#include "replacement_metrics_builder.h"

ReplacementMetricsBuilder& ReplacementMetricsBuilder::with_generation(
    long generation) {
  generation_ = generation;
  return *this;
}

ReplacementMetricsBuilder& ReplacementMetricsBuilder::with_num_teams(
    std::size_t num_teams) {
  num_teams_ = num_teams;
  return *this;
}

ReplacementMetricsBuilder& ReplacementMetricsBuilder::with_num_programs(
    std::size_t num_programs) {
  num_programs_ = num_programs;
  return *this;
}

ReplacementMetricsBuilder& ReplacementMetricsBuilder::with_memory_size(
    std::size_t memory_size) {
  memory_size_ = memory_size;
  return *this;
}

ReplacementMetricsBuilder& ReplacementMetricsBuilder::with_num_new_teams(
    int num_new_teams) {
  num_new_teams_ = num_new_teams;
  return *this;
}

ReplacementMetricsBuilder& ReplacementMetricsBuilder::with_num_elite_teams(
    uint_fast32_t num_elite_teams) {
  num_elite_teams_ = num_elite_teams;
  return *this;
}

ReplacementMetrics ReplacementMetricsBuilder::build() const {
  return ReplacementMetrics(generation_, num_teams_, num_programs_,
                            memory_size_, num_elite_teams_, num_new_teams_);
}