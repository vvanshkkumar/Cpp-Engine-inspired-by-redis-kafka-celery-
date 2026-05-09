#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <future>
#include <stdexcept>

class ThreadPool {
public:
    explicit ThreadPool(size_t numThreads);
    ~ThreadPool();

    template<typename F, typename... Args>
    auto submit(F&& f, Args&&... args)
        -> std::future<std::invoke_result_t<F, Args...>>;

    void shutdown();
    size_t queueSize() const;

private:
    void workerLoop();

    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> queue_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic<bool> stop_{false};
};

template<typename F, typename... Args>
auto ThreadPool::submit(F&& f, Args&&... args)
    -> std::future<std::invoke_result_t<F, Args...>> {

    using R = std::invoke_result_t<F, Args...>;

    auto task = std::make_shared<std::packaged_task<R()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    auto future = task->get_future();

    {
        std::unique_lock lock(mutex_);

        if (stop_.load())
            throw std::runtime_error("ThreadPool stopped");

        queue_.push([task]() { (*task)(); });
    }

    cv_.notify_one();

    return future;
}