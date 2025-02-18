#include "replacement_logger.h"
#include "core/event_dispatcher.h"
#include "core/event_types.h"
#include "storage/replacement/replacement_storage.h"

void ReplacementLogger::init() {
    EventDispatcher<ReplacementMetrics>::instance().subscribe(
        EventType::REPLACEMENT,
        [this](const ReplacementMetrics& data) { handleEvent(data); }
    );
}

void ReplacementLogger::handleEvent(const ReplacementMetrics& metrics) {
    ReplacementStorage::instance().append(metrics);
}