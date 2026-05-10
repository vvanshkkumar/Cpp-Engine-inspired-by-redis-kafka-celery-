#include <iostream>
#include <thread>
#include "src/pubsub/EventBus.h"
#include "src/taskqueue/ThreadPool.h"

int main() {
    std::cout << "\n=== Pub-Sub Demo ===\n\n";

    ThreadPool pool(4);

    EventBus bus(pool);

    bus.subscribe("orders", [](const Message& m) {
        std::cout
            << "[Warehouse] Got order: "
            << m.payload
            << "\n";
    });

    bus.subscribe("orders", [](const Message& m) {
        std::cout
            << "[Email] Sending confirmation for: "
            << m.payload
            << "\n";
    });

    bus.subscribe("alerts", [](const Message& m) {
        std::cout
            << "[ALERT] "
            << m.payload
            << "\n";
    });

    std::cout
        << "Publishing 3 messages...\n\n";

    bus.publish({
        "orders",
        "Order #101 - iPhone 15",
        "shop"
    });

    bus.publish({
        "orders",
        "Order #102 - MacBook M1",
        "shop"
    });

    bus.publish({
        "alerts",
        "Low stock: AirPods (2 left)",
        "inventory"
    });

    std::this_thread::sleep_for(
        std::chrono::milliseconds(500)
    );

    std::cout
        << "\n=== Demo Complete! ===\n";

    return 0;
}