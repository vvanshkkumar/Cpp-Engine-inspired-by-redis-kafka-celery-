#include "ThreadPool.h"

ThreadPool::ThreadPool(size_t numThreads) {
    for (size_t i = 0; i < numThreads; ++i)
        workers_.emplace_back(&ThreadPool::workerLoop, this);
}

ThreadPool::~ThreadPool() {
    shutdown();
}

void ThreadPool::workerLoop() {
    while (true) {
        std::function<void()> task;

        {
            std::unique_lock lock(mutex_);

            cv_.wait(lock, [this] {
                return !queue_.empty() || stop_.load();
            });

            if (stop_.load() && queue_.empty())
                return;

            task = std::move(queue_.front());
            queue_.pop();
        }

        task();
    }
}

void ThreadPool::shutdown() {
    stop_.store(true);

    cv_.notify_all();

    for (auto& t : workers_)
        if (t.joinable())
            t.join();
}

size_t ThreadPool::queueSize() const {
    std::unique_lock lock(mutex_);
    return queue_.size();
}