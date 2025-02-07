#include "storage/csv_storage.h"
#include "core/event_dispatcher.h"
#include "metrics/replacement/replacement_metrics.h"


class ReplacementLogger {
    public:
        ReplacementLogger() = default;
        void init(); 
    
    private:
        void handleEvent(const ReplacementMetrics& metrics);
};
