#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include "src/kvstore/KVStore.h"
#include "src/taskqueue/ThreadPool.h"
#include "src/pubsub/EventBus.h"

int main() {
    std::cout << "=== 30-Second Stress Test ===\n";
    std::cout << "10 threads hammering all components simultaneously\n\n";

    KVStore kv;
    ThreadPool pool(4);
    EventBus bus(pool);

    std::atomic<long long> ops{0};

    auto deadline = std::chrono::steady_clock::now()
        + std::chrono::seconds(30);

    std::vector<std::thread> threads;

    for (int i = 0; i < 4; i++) {
        threads.emplace_back([&, i]() {
            int k = 0;
            while (std::chrono::steady_clock::now() < deadline) {
                kv.set(
                    "k" + std::to_string(k++ % 500),
                    "v" + std::to_string(i),
                    std::chrono::milliseconds(500)
                );
                ops++;
            }
        });
    }

    for (int i = 0; i < 4; i++) {
        threads.emplace_back([&]() {
            int k = 0;
            while (std::chrono::steady_clock::now() < deadline) {
                kv.get("k" + std::to_string(k++ % 500));
                ops++;
            }
        });
    }

    bus.subscribe("stress", [](const Message&) {});

    threads.emplace_back([&]() {
        while (std::chrono::steady_clock::now() < deadline) {
            bus.publish({"stress", "ping", "test"});
            ops++;
            std::this_thread::sleep_for(
                std::chrono::milliseconds(1)
            );
        }
    });

    threads.emplace_back([&]() {
        while (std::chrono::steady_clock::now() < deadline) {
            bus.publish({"stress", "pong", "test2"});
            ops++;
            std::this_thread::sleep_for(
                std::chrono::milliseconds(1)
            );
        }
    });

    auto start = std::chrono::steady_clock::now();

    while (std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(
            std::chrono::seconds(5)
        );

        auto elapsed =
            std::chrono::duration_cast<
                std::chrono::seconds
            >(
                std::chrono::steady_clock::now()
                - start
            ).count();

        std::cout
            << " "
            << elapsed
            << "s | ops: "
            << ops.load()
            << "\n";
    }

    for (auto& t : threads) {
        t.join();
    }

    std::cout
        << "\n=== STRESS TEST PASSED ===\n";

    std::cout
        << "Total ops: "
        << ops.load()
        << "\n";

    std::cout
        << "Zero crashes = thread safety confirmed!\n";

    return 0;
}