#ifndef REMOVAL_METRICS_H
#define REMOVAL_METRICS_H

#include <cstddef>
#include <cstdint>


struct RemovalMetrics {
    RemovalMetrics(long generation, std::size_t num_teams,
                        std::size_t num_programs, int num_root_programs, uint_fast32_t num_elite_teams,
                        int num_deleted, int num_old_deleted, double percent_old_deleted)
        :   generation(generation),
            num_teams(num_teams),
            num_programs(num_programs),
            num_root_programs(num_root_programs),
            num_elite_teams(num_elite_teams),
            num_deleted(num_deleted),
            num_old_deleted(num_old_deleted),
            percent_old_deleted(percent_old_deleted) {}    

    const long generation;
    const std::size_t num_teams;
    const std::size_t num_programs;
    const int num_root_programs;
    const uint_fast32_t num_elite_teams;
    const int num_deleted;
    const int num_old_deleted;
    const double percent_old_deleted;  
};
#endif