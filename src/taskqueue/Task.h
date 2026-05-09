#pragma once

#include <functional>
#include <string>

enum class TaskState { PENDING, RUNNING, DONE, FAILED };

struct Task {
    using Fn = std::function<void()>;

    std::string id;
    Fn fn;
    TaskState state{TaskState::PENDING};
    int retryCount{0};
    int maxRetries{3};
    std::string errorMessage;
};