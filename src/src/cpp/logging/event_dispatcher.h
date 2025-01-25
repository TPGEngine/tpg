// EventDispatcher.h
#pragma once
#include "event_types.h"
#include <functional>
#include <map>
#include <vector>

class EventDispatcher {
public:
    using Callback = std::function<void(const std::map<std::string, std::string>&)>;
    
    static EventDispatcher& instance();
    void subscribe(EventType type, Callback cb);
    void notify(EventType type, const std::map<std::string, std::string>& data);
    EventDispatcher(const EventDispatcher&) = delete;
    EventDispatcher& operator=(const EventDispatcher&) = delete;

private:
    EventDispatcher() = default;
    std::map<EventType, std::vector<Callback>> listeners_;
};
