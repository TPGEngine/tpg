#include "storage/csv_storage.h"
#include "core/event_dispatcher.h"
#include "metrics/selection/selection_metrics.h"


class SelectionLogger {
    public:
        SelectionLogger() = default;
        void init(); 
    
    private:
        void handleEvent(const SelectionMetrics& metrics);
};
