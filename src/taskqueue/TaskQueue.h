#pragma once

#include <string>
#include <functional>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <cmath>
#include <random>
#include <thread>
#include <stdexcept>

#include "ThreadPool.h"
#include "Task.h"
#include "RetryPolicy.h"

class TaskQueue {
public:
    explicit TaskQueue(ThreadPool& pool,
                       RetryPolicy policy = {});

    std::string submit(Task::Fn fn,
                       const std::string& name = "");

    TaskState getStatus(const std::string& id) const;

private:
    void execute(std::string id);

    ThreadPool& pool_;
    RetryPolicy policy_;
    mutable std::mutex mutex_;
    std::unordered_map<std::string, Task> tasks_;
    std::atomic<size_t> counter_{0};
};

inline TaskQueue::TaskQueue(ThreadPool& pool,
                            RetryPolicy policy)
    : pool_(pool), policy_(policy) {}

inline std::string TaskQueue::submit(Task::Fn fn,
                                     const std::string& name) {

    std::string id =
        (name.empty() ? "task" : name)
        + "_" + std::to_string(counter_++);

    Task t;
    t.id = id;
    t.fn = fn;
    t.maxRetries = policy_.maxRetries;
    t.state = TaskState::PENDING;

    {
        std::unique_lock lock(mutex_);
        tasks_[id] = t;
    }

    pool_.submit([this, id]() {
        execute(id);
    });

    return id;
}

inline TaskState TaskQueue::getStatus(
    const std::string& id) const {

    std::unique_lock lock(mutex_);

    auto it = tasks_.find(id);

    if (it == tasks_.end())
        throw std::runtime_error("Task not found");

    return it->second.state;
}

inline void TaskQueue::execute(std::string id) {
    Task task;

    {
        std::unique_lock lock(mutex_);
        task = tasks_[id];
        tasks_[id].state = TaskState::RUNNING;
    }

    try {
        task.fn();

        std::unique_lock lock(mutex_);
        tasks_[id].state = TaskState::DONE;

    } catch (const std::exception& e) {

        std::unique_lock lock(mutex_);

        int retries = tasks_[id].retryCount;

        if (retries < tasks_[id].maxRetries) {

            tasks_[id].retryCount++;
            tasks_[id].state = TaskState::PENDING;

            lock.unlock();

            int ms = static_cast<int>(
                policy_.baseDelay.count() *
                std::pow(policy_.multiplier, retries)
            );

            if (policy_.jitter) {
                std::mt19937 rng(std::random_device{}());
                ms += std::uniform_int_distribution<int>(-30, 30)(rng);
            }

            std::this_thread::sleep_for(
                std::chrono::milliseconds(ms)
            );

            pool_.submit([this, id]() {
                execute(id);
            });

        } else {
            tasks_[id].state = TaskState::FAILED;
            tasks_[id].errorMessage = e.what();
        }
    }
}