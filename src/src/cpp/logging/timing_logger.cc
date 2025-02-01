#include "timing_logger.h"
#include "event_dispatcher.h"
#include "event_types.h"
#include "timing_storage.h"


TimingLogger::TimingLogger() {
    EventDispatcher<TimingMetrics>::instance().subscribe(
        EventType::TMS,
        [this](const TimingMetrics& data) { handleEvent(data); }
    );
}

void TimingLogger::handleEvent(const TimingMetrics& metrics) {
    TimingStorage::instance().append(metrics);
}