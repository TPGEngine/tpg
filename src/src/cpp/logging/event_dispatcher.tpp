#include <event_dispatcher.h>

template <typename MetricsType>
EventDispatcher<MetricsType>& EventDispatcher<MetricsType>::instance() {
    static EventDispatcher<MetricsType> instance;
    return instance;
}

template <typename MetricsType>
void EventDispatcher<MetricsType>::subscribe(EventType type, Callback cb) {
    listeners_[type].push_back(cb);
}

template <typename MetricsType>
void EventDispatcher<MetricsType>::notify(EventType type, const MetricsType& data) {
    auto it = listeners_.find(type);
    if (it != listeners_.end()) {
        for (auto& cb : it->second) {
            cb(data);
        }
    }
}

