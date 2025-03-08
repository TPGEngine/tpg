#ifndef REMOVAL_METRICS_BUILDER_H
#define REMOVAL_METRICS_BUILDER_H

#include "removal_metrics.h"
#include <cstdint>

class RemovalMetricsBuilder {
public:
  RemovalMetricsBuilder& with_generation(long generation);
  RemovalMetricsBuilder& with_num_teams(std::size_t num_teams);
  RemovalMetricsBuilder& with_num_programs(std::size_t num_programs);
  RemovalMetricsBuilder& with_num_root_programs(int num_root_programs);
  RemovalMetricsBuilder& with_num_elite_teams(uint_fast32_t num_elite_teams);
  RemovalMetricsBuilder& with_num_deleted(int num_deleted);
  RemovalMetricsBuilder& with_num_old_deleted(int num_old_deleted);
  RemovalMetricsBuilder& with_percent_old_deleted(double percent_old_deleted);

  RemovalMetrics build() const;

private:
  long generation_;
  std::size_t num_teams_;
  std::size_t num_programs_;
  int num_root_programs_;
  uint_fast32_t num_elite_teams_;
  int num_deleted_;
  int num_old_deleted_;
  double percent_old_deleted_;
};

#endif
