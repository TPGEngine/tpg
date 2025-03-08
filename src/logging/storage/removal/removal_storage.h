#pragma once
#include "storage/csv_storage.h"
#include "metrics/removal/removal_metrics.h"


class RemovalStorage : public CSVStorage<RemovalMetrics> {
public:
    static RemovalStorage& instance() {
        static RemovalStorage instance;
        return instance;
    }

    void init(const int& seed_tpg, const int& pid) override;
    void append(const RemovalMetrics& metrics) override;

    // Disable copy and move operations.
    RemovalStorage(const RemovalStorage&) = delete;
    RemovalStorage& operator=(const RemovalStorage&) = delete;
    RemovalStorage(RemovalStorage&&) = delete;
    RemovalStorage& operator=(RemovalStorage&&) = delete;
    ~RemovalStorage() override = default;

private:
    RemovalStorage() = default;
};
