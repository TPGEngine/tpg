#pragma once
#include "storage/csv_storage.h"
#include "metrics/tms/timing_metrics.h"


class TimingStorage : public CSVStorage<TimingMetrics> {
public:
    static TimingStorage& instance() {
        static TimingStorage instance;
        return instance;
    }

    void init(const int& seed_tpg, const int& pid) override;
    void append(const TimingMetrics& metrics) override;

    // Disable copy and move operations.
    TimingStorage(const TimingStorage&) = delete;
    TimingStorage& operator=(const TimingStorage&) = delete;
    TimingStorage(TimingStorage&&) = delete;
    TimingStorage& operator=(TimingStorage&&) = delete;
    ~TimingStorage() override = default;

private:
    TimingStorage() = default;
};
