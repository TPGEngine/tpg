#include "timing_metrics.h"
#include "timing_metrics_builder.h"

TimingMetrics::TimingMetrics(const TimingMetricsBuilder& builder)
    : generation(builder.get_generation()),
      total_generation_time(builder.get_total_generation_time()),
      evaluation_time(builder.get_evaluation_time()),
      generate_teams_time(builder.get_generate_teams_time()),
      set_elite_teams_time(builder.get_set_elite_teams_time()),
      select_teams_time(builder.get_select_teams_time()),
      report_time(builder.get_report_time()),
      modes_time(builder.get_modes_time()),
      lost_time(builder.get_lost_time()) {
}