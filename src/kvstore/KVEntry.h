#pragma once

#include <string>
#include <chrono>
#include <optional>

struct KVEntry {

    std::string value;

    std::optional<std::chrono::steady_clock::time_point> expiry;

    bool isExpired() const {

        if (!expiry.has_value()) {
            return false;
        }

        return std::chrono::steady_clock::now()
            >= expiry.value();
    }
};