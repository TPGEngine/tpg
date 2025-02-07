#include "storage/csv_storage.h"
#include "core/event_dispatcher.h"
#include "metrics/removal/removal_metrics.h"


class RemovalLogger {
    public:
        RemovalLogger() = default;
        void init(); 
    
    private:
        void handleEvent(const RemovalMetrics& metrics);
};
