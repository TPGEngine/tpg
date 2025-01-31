#include <selection_logger.h>
#include <event_dispatcher.h>
#include <event_types.h>
#include <mta_storage.h>

SelectionLogger::SelectionLogger() {
    EventDispatcher::instance().subscribe(
        EventType::SELECTION,
        [this](const std::map<std::string, std::string>& data) { handleEvent(data); }
    );
}

void SelectionLogger::handleEvent(const std::map<std::string, std::string>& data) {
    MTAStorage::instance().append({
        {"type", data.at("type")},
        {"generation", data.at("generation")},
        {"best_fitness", data.at("best_fitness")},
        {"team_id", data.at("team_id")}
    });
}