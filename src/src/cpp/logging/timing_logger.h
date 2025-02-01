#include <csv_storage.h>
#include <event_dispatcher.h>
#include "timing_metrics.h"


class TimingLogger {
    public:
        TimingLogger();
    
    private:
        void handleEvent(const TimingMetrics& metrics);
};
