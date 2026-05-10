#pragma once

#include <string>
#include <thread>

#include "../kvstore/KVStore.h"
#include "../kvstore/TTLManager.h"
#include "../taskqueue/ThreadPool.h"
#include "../taskqueue/TaskQueue.h"
#include "../pubsub/EventBus.h"
#include "../persistence/WAL.h"

class Engine {
public:
    struct Config {
        size_t threadPoolSize{4};

        std::chrono::milliseconds
            ttlInterval{100};

        std::string walPath{
            "wal.log"
        };
    };

    explicit Engine(
        Config cfg 
    );

    ~Engine();

    void start();

    void stop();

    std::string execute(
        const std::string& raw
    );

    KVStore& kv() {
        return kv_;
    }

    EventBus& bus() {
        return bus_;
    }

    TaskQueue& queue() {
        return queue_;
    }

private:
    void recoverFromWAL();

    Config cfg_;

    WAL wal_;

    KVStore kv_;

    TTLManager ttl_;

    ThreadPool pool_;

    TaskQueue queue_;

    EventBus bus_;
};