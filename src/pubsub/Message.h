#pragma once

#include <string>
#include <chrono>

struct Message {
    std::string topic;
    std::string payload;
    std::string publisherId;

    std::chrono::steady_clock::time_point timestamp{
        std::chrono::steady_clock::now()
    };
}; 