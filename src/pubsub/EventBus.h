#pragma once

#include <string>
#include <functional>
#include <unordered_map>
#include <vector>
#include <shared_mutex>
#include <atomic>
#include <algorithm>

#include "Message.h"
#include "../taskqueue/ThreadPool.h"

class EventBus {
public:
    using Callback =
        std::function<void(const Message&)>;

    using SubId = size_t;

    explicit EventBus(ThreadPool& pool);

    SubId subscribe(
        const std::string& topic,
        Callback cb
    );

    void unsubscribe(
        const std::string& topic,
        SubId id
    );

    void publish(const Message& msg);

    size_t subscriberCount(
        const std::string& topic
    ) const;

private:
    struct Sub {
        SubId id;
        Callback cb;
    };

    ThreadPool& pool_;

    mutable std::shared_mutex mutex_;

    std::unordered_map<
        std::string,
        std::vector<Sub>
    > topics_;

    std::atomic<SubId> nextId_{0};
};