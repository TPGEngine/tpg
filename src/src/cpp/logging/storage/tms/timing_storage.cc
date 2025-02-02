#include "timing_storage.h"
#include <iomanip>

void TimingStorage::init(const int& seed_tpg, const int& pid) {
    std::stringstream filename;
    filename << "tms." << seed_tpg << "." << pid << ".csv";

    file_.open(filename.str());
    file_ << "generation,generation_time,evaluation_time,generate_teams_time,set_elite_teams_time,select_teams_time,report_time,modes_time,lost_time\n";
    file_.flush();
}

void TimingStorage::append(const TimingMetrics& metrics) {
    file_ << metrics.generation << ","
            << std::fixed << std::setprecision(6) << metrics.total_generation_time << ","
            << metrics.evaluation_time << ","
            << metrics.generate_teams_time << ","
            << metrics.set_elite_teams_time << ","
            << metrics.select_teams_time << ","
            << metrics.report_time << ","
            << metrics.modes_time << ","
            << metrics.lost_time << "\n";
    file_.flush();
}
