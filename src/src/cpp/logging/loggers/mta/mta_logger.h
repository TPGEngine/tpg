#include "storage/csv_storage.h"
#include "core/event_dispatcher.h"
#include "metrics/mta/mta_metrics.h"


class MTALogger {
    public:
        MTALogger() = default;
        void init(); 
    
    private:
        void handleEvent(const MTAMetrics& metrics);
};
