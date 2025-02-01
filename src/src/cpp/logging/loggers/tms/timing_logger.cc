#include "timing_logger.h"
#include "core/event_dispatcher.h"
#include "core/event_types.h"
#include "storage/tms/timing_storage.h"


TimingLogger::TimingLogger() {
    EventDispatcher<TimingMetrics>::instance().subscribe(
        EventType::TMS,
        [this](const TimingMetrics& data) { handleEvent(data); }
    );
}

void TimingLogger::handleEvent(const TimingMetrics& metrics) {
    TimingStorage::instance().append(metrics);
}