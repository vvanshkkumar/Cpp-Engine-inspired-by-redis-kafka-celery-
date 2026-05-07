#pragma once

#include <string>
#include <optional>
#include <unordered_map>
#include <shared_mutex>
#include <chrono>
#include <vector>

#include "KVEntry.h"

class KVStore {
public:
    void set(
        const std::string& key,
        const std::string& value,
        std::optional<std::chrono::milliseconds> ttl = std::nullopt
    );

    std::optional<std::string> get(
        const std::string& key
    );

    bool del(
        const std::string& key
    );

    void purgeExpired();

    size_t size() const;

private:
    mutable std::shared_mutex mutex_;

    std::unordered_map<std::string, KVEntry> store_;
};