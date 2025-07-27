#pragma once
#include <functional>
#include <queue>

#include "Priority.h"

struct Task {
    std::function<void()> task;
    Priority priority;

    bool operator<(const Task& other) const {
        return static_cast<int>(priority) < static_cast<int>(other.priority);
    }
};
