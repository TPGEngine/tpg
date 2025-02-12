#include "selection_logger.h"
#include "core/event_dispatcher.h"
#include "core/event_types.h"
#include "storage/selection/selection_storage.h"

void SelectionLogger::init() {
    EventDispatcher<SelectionMetrics>::instance().subscribe(
        EventType::SELECTION,
        [this](const SelectionMetrics& data) { handleEvent(data); }
    );
}

void SelectionLogger::handleEvent(const SelectionMetrics& metrics) {
    SelectionStorage::instance().append(metrics);
}