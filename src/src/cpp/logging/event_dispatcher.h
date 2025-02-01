// EventDispatcher.h
#pragma once
#include "event_types.h"
#include <functional>
#include <map>
#include <vector>
#include <mta_metrics.h>

template<typename T>
class EventDispatcher {
public:
    using Callback = std::function<void(const T&)>;
    
    static EventDispatcher& instance();
    void subscribe(EventType type, Callback cb);
    void notify(EventType type, const T& data);
    EventDispatcher(const EventDispatcher&) = delete;
    EventDispatcher& operator=(const EventDispatcher&) = delete;

private:
    EventDispatcher() = default;
    std::map<EventType, std::vector<Callback>> listeners_;
};
