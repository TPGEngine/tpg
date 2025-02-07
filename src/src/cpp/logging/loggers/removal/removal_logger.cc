#include "removal_logger.h"
#include "core/event_dispatcher.h"
#include "core/event_types.h"
#include "storage/removal/removal_storage.h"

void RemovalLogger::init() {
    EventDispatcher<RemovalMetrics>::instance().subscribe(
        EventType::REMOVAL,
        [this](const RemovalMetrics& data) { handleEvent(data); }
    );
}

void RemovalLogger::handleEvent(const RemovalMetrics& metrics) {
    RemovalStorage::instance().append(metrics);
}