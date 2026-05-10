#pragma once

#include <string>
#include "../kvstore/KVStore.h"
#include "../pubsub/EventBus.h"
#include "../taskqueue/ThreadPool.h"
#include "../taskqueue/TaskQueue.h"
#include "../persistence/WAL.h"
#include "../network/CommandParser.h"

class Engine {
public:
    Engine();

    std::string execute(
        const std::string& rawCommand
    );

private:
    KVStore kv_;
    ThreadPool pool_;
    TaskQueue taskQueue_;
    EventBus bus_;
    WAL wal_;
};