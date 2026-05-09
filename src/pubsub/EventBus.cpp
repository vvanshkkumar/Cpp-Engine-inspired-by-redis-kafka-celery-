#include "EventBus.h"

EventBus::EventBus(ThreadPool& pool)
    : pool_(pool) {}

EventBus::SubId EventBus::subscribe(
    const std::string& topic,
    Callback cb
) {
    SubId id = nextId_++;

    std::unique_lock lock(mutex_);

    topics_[topic].push_back({
        id,
        cb
    });

    return id;
}

void EventBus::unsubscribe(
    const std::string& topic,
    SubId id
) {
    std::unique_lock lock(mutex_);

    auto it = topics_.find(topic);

    if (it == topics_.end()) {
        return;
    }

    auto& subs = it->second;

    subs.erase(
        std::remove_if(
            subs.begin(),
            subs.end(),
            [id](const Sub& s) {
                return s.id == id;
            }
        ),
        subs.end()
    );
}

void EventBus::publish(
    const Message& msg
) {
    std::vector<Callback> cbs;

    {
        std::shared_lock lock(mutex_);

        auto it = topics_.find(msg.topic);

        if (it == topics_.end()) {
            return;
        }

        for (auto& s : it->second) {
            cbs.push_back(s.cb);
        }
    }

    for (auto& cb : cbs) {
        pool_.submit([cb, msg]() {
            cb(msg);
        });
    }
}

size_t EventBus::subscriberCount(
    const std::string& topic
) const {
    std::shared_lock lock(mutex_);

    auto it = topics_.find(topic);

    if (it == topics_.end()) {
        return 0;
    }

    return it->second.size();
}