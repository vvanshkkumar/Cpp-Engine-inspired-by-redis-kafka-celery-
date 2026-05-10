#include <iostream>
#include <thread>
#include "src/kvstore/KVStore.h"
#include "src/kvstore/TTLManager.h"

int main() {
    std::cout << "\n=== KV Store Demo ===\n\n";

    KVStore kv;

    TTLManager ttl(kv, std::chrono::milliseconds(200));
    ttl.start();

    kv.set("name", "Alice");
    kv.set("city", "Delhi");

    auto name = kv.get("name");

    std::cout << "SET name=Alice, city=Delhi\n";
    std::cout << "GET name -> " << (name ? *name : "null") << "\n";

    kv.set("session", "abc123", std::chrono::milliseconds(2000));

    std::cout << "\nSET session=abc123 with TTL=2 seconds\n";

    auto s1 = kv.get("session");

    std::cout << "GET session (now) -> "
              << (s1 ? *s1 : "null") << "\n";

    std::cout << "Waiting 3 seconds...\n";

    std::this_thread::sleep_for(std::chrono::seconds(3));

    auto s2 = kv.get("session");

    std::cout << "GET session (after 3s) -> "
              << (s2 ? *s2 : "(null - key expired!)") << "\n";

    kv.del("name");

    auto after = kv.get("name");

    std::cout << "\nDEL name\n";

    std::cout << "GET name -> "
              << (after ? *after : "(null - deleted!)") << "\n";

    std::cout << "\nKV Store size: "
              << kv.size() << " keys\n";

    ttl.stop();

    std::cout << "\n=== Demo Complete! ===\n";

    return 0;
}