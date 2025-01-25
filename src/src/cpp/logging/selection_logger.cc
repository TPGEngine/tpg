#include <selection_logger.h>
#include <event_dispatcher.h>
#include <event_types.h>

SelectionLogger::SelectionLogger() {
    EventDispatcher::instance().subscribe(
        EventType::SELECTION,
        [this](const auto& data) { handleEvent(data); }
    );
}

void SelectionLogger::handleEvent(const std::map<std::string, std::string>& data) {
    CSVStorage::instance().append({
        {"generation", data.at("generation")},
        {"best_fitness", data.at("best_fitness")},
        {"team_id", data.at("team_id")}
    });
}