// replacement_metrics_builder.h
#ifndef REPLACEMENT_METRICS_BUILDER_H
#define REPLACEMENT_METRICS_BUILDER_H

#include "replacement_metrics.h"
#include <cstdint>

class ReplacementMetricsBuilder {
public:
  ReplacementMetricsBuilder& with_generation(long generation);
  ReplacementMetricsBuilder& with_num_teams(std::size_t num_teams);
  ReplacementMetricsBuilder& with_num_programs(std::size_t num_programs);
  ReplacementMetricsBuilder& with_memory_size(std::size_t memory_size);
  ReplacementMetricsBuilder& with_num_elite_teams(uint_fast32_t num_elite_teams);
  ReplacementMetricsBuilder& with_num_new_teams(int num_new_teams);

  ReplacementMetrics build() const;

private:
  long generation_;
  std::size_t num_teams_;
  std::size_t num_programs_;
  std::size_t memory_size_;
  uint_fast32_t num_elite_teams_;
  int num_new_teams_;
};

#endif
