#pragma once
#include "storage/csv_storage.h"
#include "metrics/mta/mta_metrics.h"


class MTAStorage : public CSVStorage<MTAMetrics> {
public:
    static MTAStorage& instance() {
        static MTAStorage instance;
        return instance;
    }

    void init(const int& seed_tpg, const int& pid) override;
    void append(const MTAMetrics& metrics) override;

    // Disable copy and move operations.
    MTAStorage(const MTAStorage&) = delete;
    MTAStorage& operator=(const MTAStorage&) = delete;
    MTAStorage(MTAStorage&&) = delete;
    MTAStorage& operator=(MTAStorage&&) = delete;

private:
    MTAStorage() = default;
};
