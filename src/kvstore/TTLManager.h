#pragma once

#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>
#include <condition_variable>

#include "KVStore.h"

class TTLManager {
public:
    explicit TTLManager(
        KVStore& store,
        std::chrono::milliseconds interval =
            std::chrono::milliseconds(100)
    );

    ~TTLManager();

    void start();

    void stop();

private:
    void run();

    KVStore& store_;

    std::chrono::milliseconds interval_;

    std::atomic<bool> running_{false};

    std::thread thread_;

    std::mutex mutex_;

    std::condition_variable cv_;
};