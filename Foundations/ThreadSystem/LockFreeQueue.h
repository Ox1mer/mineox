#pragma once

#include <vector>
#include <atomic>
#include <optional>

#include "Task.h"

class LockFreeQueue {
public:
    LockFreeQueue(size_t capacity = 1024, size_t maxTasks = 200)
        : buffer(capacity + 1), maxTasks(maxTasks)
    {}

    bool push(const Task& item) {
        size_t nextHead = (head + 1) % buffer.size();

        if (nextHead == tail.load(std::memory_order_acquire)) {
            tail.store((tail.load() + 1) % buffer.size(), std::memory_order_release);
        }

        buffer[head] = item;
        head = nextHead;

        size_t currentSize = sizeApprox();
        if (currentSize > maxTasks) {
            tail.store((tail.load() + (currentSize - maxTasks)) % buffer.size(), std::memory_order_release);
        }

        return true;
    }

    std::optional<Task> pop() {
        if (tail.load(std::memory_order_acquire) == head) {
            return std::nullopt;
        }

        Task item = buffer[tail];
        tail.store((tail.load() + 1) % buffer.size(), std::memory_order_release);
        return item;
    }

    size_t sizeApprox() const {
        if (head >= tail) {
            return head - tail.load();
        } else {
            return buffer.size() - tail.load() + head;
        }
    }

private:
    std::vector<Task> buffer;
    size_t head = 0;
    std::atomic<size_t> tail = 0;

    size_t maxTasks;
};
