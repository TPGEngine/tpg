#include "timing_logger.h"
#include "core/event_dispatcher.h"
#include "core/event_types.h"
#include "storage/timing/timing_storage.h"


void TimingLogger::init() {
    EventDispatcher<TimingMetrics>::instance().subscribe(
        EventType::TMS,
        [this](const TimingMetrics& data) { handleEvent(data); }
    );
}

void TimingLogger::handleEvent(const TimingMetrics& metrics) {
    TimingStorage::instance().append(metrics);
}