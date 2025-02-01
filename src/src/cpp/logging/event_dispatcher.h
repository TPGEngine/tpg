// EventDispatcher.h
#pragma once
#include "event_types.h"
#include <functional>
#include <map>
#include <vector>

template <typename MetricsType>
class EventDispatcher {
public:
    using Callback = std::function<void(const MetricsType&)>;
    
    static EventDispatcher<MetricsType>& instance();
    void subscribe(EventType type, Callback cb);
    void notify(EventType type, const MetricsType& data);
    EventDispatcher(const EventDispatcher&) = delete;
    EventDispatcher& operator=(const EventDispatcher&) = delete;

private:
    EventDispatcher() = default;
    std::map<EventType, std::vector<Callback>> listeners_;
};

#include "event_dispatcher.tpp"
