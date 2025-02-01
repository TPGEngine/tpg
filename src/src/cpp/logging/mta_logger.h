#include <csv_storage.h>
#include <event_dispatcher.h>


class MTALogger {
    public:
        MTALogger();
    
    private:
        void handleEvent(const MTAMetrics& metrics);
};
