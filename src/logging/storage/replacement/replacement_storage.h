#pragma once
#include "storage/csv_storage.h"
#include "metrics/replacement/replacement_metrics.h"


class ReplacementStorage : public CSVStorage<ReplacementMetrics> {
public:
    static ReplacementStorage& instance() {
        static ReplacementStorage instance;
        return instance;
    }

    void init(const int& seed_tpg, const int& pid) override;
    void append(const ReplacementMetrics& metrics) override;

    // Disable copy and move operations.
    ReplacementStorage(const ReplacementStorage&) = delete;
    ReplacementStorage& operator=(const ReplacementStorage&) = delete;
    ReplacementStorage(ReplacementStorage&&) = delete;
    ReplacementStorage& operator=(ReplacementStorage&&) = delete;
    ~ReplacementStorage() override = default;

private:
    ReplacementStorage() = default;
};
