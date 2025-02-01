#include "event_dispatcher.h"

    
EventDispatcher& EventDispatcher::instance() {
    static EventDispatcher instance;
    return instance;
}

void EventDispatcher::subscribe(EventType type, Callback cb) {
    listeners_[type].push_back(cb);
}

void EventDispatcher::notify(EventType type, const T& data) {
    if (listeners_.find(type) != listeners_.end()) {
        for (auto& cb : listeners_[type]) {
            cb(data);
        }
    }
}

