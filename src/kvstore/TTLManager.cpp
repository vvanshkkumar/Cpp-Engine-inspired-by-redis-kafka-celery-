#include "TTLManager.h"

TTLManager::TTLManager(
    KVStore& store,
    std::chrono::milliseconds interval
)
    : store_(store),
      interval_(interval) {}

TTLManager::~TTLManager() {
    stop();
}

void TTLManager::start() {
    running_.store(true);

    thread_ = std::thread(&TTLManager::run, this);
}

void TTLManager::stop() {
    if (running_.exchange(false)) {
        cv_.notify_all();

        if (thread_.joinable()) {
            thread_.join();
        }
    }
}

void TTLManager::run() {
    while (running_.load()) {
        std::unique_lock lock(mutex_);

        cv_.wait_for(
            lock,
            interval_,
            [this] {
                return !running_.load();
            }
        );

        if (running_.load()) {
            store_.purgeExpired();
        }
    }
}