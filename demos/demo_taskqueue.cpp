#include <iostream>
#include <atomic>
#include <thread>
#include <stdexcept>
#include "src/taskqueue/ThreadPool.h"
#include "src/taskqueue/TaskQueue.h"

int main() {
    std::cout << "\n=== Task Queue Demo ===\n\n";

    ThreadPool pool(4);

    RetryPolicy policy;
    policy.maxRetries = 3;
    policy.baseDelay = std::chrono::milliseconds(300);

    TaskQueue queue(pool, policy);

    auto id1 = queue.submit([]() {
        std::cout << "[Task 1] Running and succeeding!\n";
    }, "easy_task");

    std::atomic<int> attempts{0};

    auto id2 = queue.submit([&attempts]() {
        int a = ++attempts;

        std::cout << "[Task 2] Attempt " << a;

        if (a < 3) {
            std::cout << " - FAILING (will retry)\n";
            throw std::runtime_error("Simulated failure");
        }

        std::cout << " - SUCCESS after retries!\n";
    }, "retry_task");

    auto id3 = queue.submit([]() {
        throw std::runtime_error("Always fails");
    }, "fail_task");

    std::this_thread::sleep_for(std::chrono::seconds(5));

    auto stateStr = [](TaskState s) -> std::string {
        if (s == TaskState::DONE) return "DONE";
        if (s == TaskState::FAILED) return "FAILED";
        if (s == TaskState::RUNNING) return "RUNNING";
        return "PENDING";
    };

    std::cout << "\n--- Final States ---\n";

    std::cout << "Task 1: "
              << stateStr(queue.getStatus(id1)) << "\n";

    std::cout << "Task 2: "
              << stateStr(queue.getStatus(id2)) << "\n";

    std::cout << "Task 3: "
              << stateStr(queue.getStatus(id3)) << "\n";

    std::cout << "\n=== Demo Complete! ===\n";

    return 0;
}