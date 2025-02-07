#ifndef REPLACEMENT_METRICS_H
#define REPLACEMENT_METRICS_H

#include <cstddef>
#include <cstdint>


struct ReplacementMetrics {
    ReplacementMetrics(long generation, std::size_t num_teams,
                        std::size_t num_programs, std::size_t memory_size,
                        uint_fast32_t num_elite_teams,
                        int num_new_teams)
        :   generation(generation),
            num_teams(num_teams),
            num_programs(num_programs),
            memory_size(memory_size),
            num_elite_teams(num_elite_teams),
            num_new_teams(num_new_teams) {}      
    const long generation;
    const std::size_t num_teams;
    const std::size_t num_programs;
    const std::size_t memory_size;
    const uint_fast32_t num_elite_teams;
    const int num_new_teams;  
};


#endif