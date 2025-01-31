#pragma once
#include "csv_storage.h"

class TimingStorage : public CSVStorage {
public:
    static TimingStorage& instance() {
        static TimingStorage instance;
        return instance;
    }

    void init(const int& seed_tpg, const int& pid) override;
    void append(const std::map<std::string, std::string>& data) override;

    TimingStorage(const TimingStorage&) = delete;
    TimingStorage& operator=(const TimingStorage&) = delete;

private:
    TimingStorage() = default;
};
