#include "storage/csv_storage.h"
#include "core/event_dispatcher.h"
#include "metrics/timing/timing_metrics.h"


class TimingLogger {
    public:
        TimingLogger() = default;
        void init();
    
    private:
        void handleEvent(const TimingMetrics& metrics);
};
