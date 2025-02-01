#include "mta_logger.h"
#include "core/event_dispatcher.h"
#include "core/event_types.h"
#include "storage/mta/mta_storage.h"

void MTALogger::init() {
    EventDispatcher<MTAMetrics>::instance().subscribe(
        EventType::MTA,
        [this](const MTAMetrics& data) { handleEvent(data); }
    );
}

void MTALogger::handleEvent(const MTAMetrics& metrics) {
    MTAStorage::instance().append(metrics);
}