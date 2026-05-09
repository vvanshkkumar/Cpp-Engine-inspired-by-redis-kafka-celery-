#pragma once

#include <chrono>

struct RetryPolicy {
    int maxRetries{3};
    std::chrono::milliseconds baseDelay{200};
    float multiplier{2.0f};
    bool jitter{true};
};