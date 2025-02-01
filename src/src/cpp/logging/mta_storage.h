#pragma once
#include "csv_storage.h"


class MTAStorage : public CSVStorage {
public:
    static MTAStorage& instance() {
        static MTAStorage instance;
        return instance;
    }

    void init(const int& seed_tpg, const int& pid) override;
    void append(const MTAMetrics& metrics) override;

    MTAStorage(const MTAStorage&) = delete;
    MTAStorage& operator=(const MTAStorage&) = delete;

private:
    MTAStorage() = default;
};
