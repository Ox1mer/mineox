#pragma once

#include <vector>
#include <thread>
#include <functional>
#include <future>
#include <atomic>
#include <stdexcept>
#include <chrono>
#include "SPMCQueue.h"
#include "Task.h"
#include "Logger.h"

class ThreadPool {
public:
    static ThreadPool& getInstance() {
        static ThreadPool instance;
        return instance;
    }

    // (Single Producer)
    template<typename F, typename... Args>
    auto enqueueChunkTask(F&& f, Args&&... args)
        -> std::future<std::invoke_result_t<F, Args...>>
    {
        using ReturnType = std::invoke_result_t<F, Args...>;

        auto taskPtr = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        if (stopping.load(std::memory_order_acquire)) {
            throw std::runtime_error("ThreadPool is stopping, cannot enqueue task");
        }

        std::future<ReturnType> future = taskPtr->get_future();

        chunkQueue.push(Task{ [taskPtr]() { (*taskPtr)(); } });

        return future;
    }

    // Перегрузка с workerIndex для совместимости
    // Позже будет убрана
    template<typename F, typename... Args>
    auto enqueueChunkTask(size_t /*workerIndex*/, F&& f, Args&&... args)
        -> std::future<std::invoke_result_t<F, Args...>>
    {
        return enqueueChunkTask(std::forward<F>(f), std::forward<Args>(args)...);
    }

private:
    ThreadPool()
        : stopping(false)
    {
        const unsigned int cpu = std::thread::hardware_concurrency();
        const size_t workersCount = (cpu > 1) ? cpu / 2 : 1;

        Logger::getInstance().Log("workers count " + std::to_string(workersCount));

        for (size_t i = 0; i < workersCount; ++i) {
            workers.emplace_back([this] { workerLoop(); });
        }
    }

    ~ThreadPool() {
        stopping.store(true, std::memory_order_release);

        for (auto& th : workers) {
            if (th.joinable()) {
                th.join();
            }
        }
    }

    void workerLoop() {
        int idleSpins = 0;
        while (!stopping.load(std::memory_order_acquire)) {
            auto optTask = chunkQueue.pop();
            if (optTask.has_value()) {
                optTask->task();
                idleSpins = 0;
            } else {
                std::this_thread::sleep_for(std::chrono::microseconds(1));
            }
        }
    }

    std::vector<std::thread> workers;
    SPMCQueue chunkQueue{1000};
    std::atomic<bool> stopping;
};
