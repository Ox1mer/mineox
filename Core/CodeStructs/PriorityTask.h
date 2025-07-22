#pragma once
#include <functional>
#include <queue>

#include "ThreadPoolPriorityTask.h"

struct PriorityTask {
    std::function<void()> task;
    Priority priority;

    bool operator<(const PriorityTask& other) const {
        return static_cast<int>(priority) < static_cast<int>(other.priority);
    }
};
