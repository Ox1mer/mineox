#pragma once

#include <vector>
#include <atomic>
#include <optional>
#include "Task.h"

class SPMCQueue {
public:
    explicit SPMCQueue(size_t maxTasks = 1000)
        : buffer(maxTasks), capacity(maxTasks), head(0), tail(0) {}

    void push(Task item) {
        size_t headIdx = head.load(std::memory_order_relaxed);
        size_t nextHead = increment(headIdx);

        if (nextHead == tail.load(std::memory_order_acquire)) {
            tail.store(increment(tail.load(std::memory_order_relaxed)), std::memory_order_release);
        }

        buffer[headIdx] = std::move(item);
        head.store(nextHead, std::memory_order_release);
    }

    std::optional<Task> pop() {
        size_t oldTail;
        size_t newTail;

        do {
            oldTail = tail.load(std::memory_order_relaxed);
            if (oldTail == head.load(std::memory_order_acquire)) {
                return std::nullopt;
            }
            newTail = increment(oldTail);
        } while (!tail.compare_exchange_weak(oldTail, newTail, std::memory_order_acquire));

        return buffer[oldTail];
    }

private:
    inline size_t increment(size_t i) const {
        return (i + 1) % capacity;
    }

    std::vector<Task> buffer;
    const size_t capacity;

    std::atomic<size_t> head; // producer writes
    std::atomic<size_t> tail; // consumers update
};
