#include <mta_logger.h>
#include <event_dispatcher.h>
#include <event_types.h>
#include <mta_storage.h>

MTALogger::MTALogger() {
    EventDispatcher::instance().subscribe(
        EventType::MTA,
        [this](const MTAMetrics& data) { handleEvent(data); }
    );
}

void MTALogger::handleEvent(const MTAMetrics& metrics) {
    MTAStorage::instance().append(metrics);
}