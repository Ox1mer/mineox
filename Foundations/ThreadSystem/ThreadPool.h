#pragma once

#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <atomic>
#include <stdexcept>
#include <memory>

#include "LockFreeQueue.h"
#include "Task.h"

class ThreadPool {
public:
    static ThreadPool& getInstance() {
        static ThreadPool instance;
        return instance;
    }

    template<typename F, typename... Args>
    auto enqueueChunkTask(F&& f, Args&&... args)
        -> std::future<typename std::invoke_result_t<F, Args...>>
    {
        using ReturnType = typename std::invoke_result_t<F, Args...>;
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        std::future<ReturnType> result = task->get_future();

        if (stopping) throw std::runtime_error("enqueueChunkTask on stopped ThreadPool");

        size_t index = chunkTaskCounter.fetch_add(1, std::memory_order_relaxed) % chunkQueues.size();
        if (!chunkQueues[index]->push(Task{ [task]() { (*task)(); } }))
            throw std::runtime_error("Chunk queue is full!");

        return result;
    }

    template<typename F, typename... Args>
    auto enqueueChunkTask(size_t workerIndex, F&& f, Args&&... args)
        -> std::future<typename std::invoke_result_t<F, Args...>>
    {
        using ReturnType = typename std::invoke_result_t<F, Args...>;
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        std::future<ReturnType> result = task->get_future();

        if (stopping) throw std::runtime_error("enqueueChunkTask on stopped ThreadPool");

        if (!chunkQueues[workerIndex]->push(Task{ [task]() { (*task)(); } }))
            throw std::runtime_error("Chunk queue is full!");

        return result;
    }

private:
    std::atomic<size_t> chunkTaskCounter = 0;

    ThreadPool() : stopping(false) {
        unsigned int cpu = std::thread::hardware_concurrency();
        size_t chunkWorkersCount = std::min<size_t>(4, cpu > 1 ? cpu - 1 : 1);
        maxGeneralWorkers = std::max<size_t>(1, cpu > chunkWorkersCount ? cpu - chunkWorkersCount : 1);

        generalWorkersCount = 1;
        workers.emplace_back([this] { generalWorker(); });

        for (size_t i = 0; i < chunkWorkersCount; ++i) {
            chunkQueues.emplace_back(std::make_unique<LockFreeQueue>(1024));
            workers.emplace_back([this, i] { chunkWorker(i); });
        }
    }

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    ~ThreadPool() {
        shutdown();
    }

    void shutdown() {
        {
            std::unique_lock lock(generalMutex);
            stopping = true;
        }
        generalCondition.notify_all();

        for (auto& thread : workers) {
            if (thread.joinable())
                thread.join();
        }
    }

    void generalWorker() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock lock(generalMutex);

                if (!generalCondition.wait_for(lock, std::chrono::seconds(5), [this] {
                        return stopping || !generalTasks.empty();
                    })) {
                    if (generalWorkersCount > 1) {
                        generalWorkersCount--;
                        return;
                    }
                    continue;
                }

                if (stopping && generalTasks.empty()) return;

                task = std::move(generalTasks.front());
                generalTasks.pop();
            }
            task();
        }
    }

    void chunkWorker(size_t workerIndex) {
        auto& queue = *chunkQueues[workerIndex];
        int spinCount = 0;

        while (!stopping) {
            auto taskOpt = queue.pop();
            if (taskOpt.has_value()) {
                taskOpt->task();
                spinCount = 0;
            } else {
                if (++spinCount < 100) {
                    std::this_thread::yield();
                } else {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    spinCount = 0;
                }
            }
        }
    }

    std::vector<std::thread> workers;

    std::queue<std::function<void()>> generalTasks;
    std::mutex generalMutex;
    std::condition_variable generalCondition;

    std::vector<std::unique_ptr<LockFreeQueue>> chunkQueues;

    std::atomic<size_t> generalWorkersCount = 0;
    size_t maxGeneralWorkers = 1;

    std::atomic<bool> stopping;
};