#pragma once
#include "csv_storage.h"
#include "timing_metrics.h"


class TimingStorage : public CSVStorage<TimingMetrics> {
public:
    static TimingStorage& instance() {
        static TimingStorage instance;
        return instance;
    }

    void init(const int& seed_tpg, const int& pid) override;
    void append(const TimingMetrics& metrics) override;

    TimingStorage(const TimingStorage&) = delete;
    TimingStorage& operator=(const TimingStorage&) = delete;

private:
    TimingStorage() = default;
};
