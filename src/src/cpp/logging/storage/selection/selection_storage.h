#pragma once
#include "storage/csv_storage.h"
#include "metrics/selection/selection_metrics.h"


class SelectionStorage : public CSVStorage<SelectionMetrics> {
public:
    static SelectionStorage& instance() {
        static SelectionStorage instance;
        return instance;
    }

    void init(const int& seed_tpg, const int& pid) override;
    void append(const SelectionMetrics& metrics) override;

    // Disable copy and move operations.
    SelectionStorage(const SelectionStorage&) = delete;
    SelectionStorage& operator=(const SelectionStorage&) = delete;
    SelectionStorage(SelectionStorage&&) = delete;
    SelectionStorage& operator=(SelectionStorage&&) = delete;
    ~SelectionStorage() override = default;

private:
    SelectionStorage() = default;
};
