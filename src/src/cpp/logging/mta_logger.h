#include <csv_storage.h>
#include <event_dispatcher.h>
#include <mta_metrics.h>


class MTALogger {
    public:
        MTALogger();
    
    private:
        void handleEvent(const MTAMetrics& metrics);
};
