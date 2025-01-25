#include "event_dispatcher.h"

    
EventDispatcher& EventDispatcher::instance() {
    static EventDispatcher instance;
    return instance;
}

void EventDispatcher::subscribe(EventType type, Callback cb) {
    std::lock_guard<std::mutex> lock(mutex_);
    listeners_[type].push_back(cb);
}

void EventDispatcher::notify(EventType type, const std::map<std::string, std::string>& data) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (listeners_.find(type) != listeners_.end()) {
        for (auto& cb : listeners_[type]) {
            cb(data);
        }
    }
}

