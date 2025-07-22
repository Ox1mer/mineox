#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <atomic>
#include <stdexcept>
#include <algorithm>

#include "PriorityTask.h"

class ThreadPool {
public:
    static ThreadPool& getInstance() {
        static ThreadPool instance;
        return instance;
    }

    template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<typename std::invoke_result_t<F, Args...>>
    {
        using ReturnType = typename std::invoke_result_t<F, Args...>;
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        std::future<ReturnType> result = task->get_future();

        {
            std::unique_lock lock(generalMutex);
            if (stopping) {
                throw std::runtime_error("enqueue on stopped ThreadPool");
            }
            generalTasks.emplace([task]() { (*task)(); });
        }
        generalCondition.notify_one();
        return result;
    }

    template<typename F, typename... Args>
    auto enqueueChunkTask(Priority priority, F&& f, Args&&... args)
        -> std::future<typename std::invoke_result_t<F, Args...>>
    {
        using ReturnType = typename std::invoke_result_t<F, Args...>;
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        std::future<ReturnType> result = task->get_future();

        {
            std::unique_lock lock(chunkMutex);
            if (stopping) {
                throw std::runtime_error("enqueueChunkTask on stopped ThreadPool");
            }

            chunkTasks.emplace_back(PriorityTask{ [task]() { (*task)(); }, priority });

            if (chunkTasks.size() > maxChunkTasks) {
                // Сортируем от большего к меньшему приоритету
                std::sort(chunkTasks.begin(), chunkTasks.end(),
                    [](const PriorityTask& a, const PriorityTask& b) {
                        return static_cast<int>(a.priority) > static_cast<int>(b.priority);
                    });
                // Удаляем лишние с конца (минимальный приоритет)
                while (chunkTasks.size() > maxChunkTasks) {
                    chunkTasks.pop_back();
                }
            }
        }
        chunkCondition.notify_one();
        return result;
    }

private:
    ThreadPool() : stopping(false) {
        unsigned int cpu = std::thread::hardware_concurrency();
        size_t totalThreads = std::max<size_t>(5, cpu);
        size_t chunkWorkersCount = (totalThreads / 2) - 1;
        size_t generalWorkersCount = totalThreads - chunkWorkersCount;

        for (size_t i = 0; i < chunkWorkersCount; ++i) {
            workers.emplace_back([this] { chunkWorker(); });
        }

        for (size_t i = 0; i < generalWorkersCount; ++i) {
            workers.emplace_back([this] { generalWorker(); });
        }

        Logger::getInstance().Log("total threads: " + std::to_string(totalThreads) +
                                  "\nchunk worker threads: " + std::to_string(chunkWorkersCount));
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
            std::unique_lock lock1(generalMutex);
            std::unique_lock lock2(chunkMutex);
            stopping = true;
        }
        generalCondition.notify_all();
        chunkCondition.notify_all();

        for (auto& thread : workers) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

    void generalWorker() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock lock(generalMutex);
                generalCondition.wait(lock, [this] { return stopping || !generalTasks.empty(); });
                if (stopping && generalTasks.empty()) return;
                task = std::move(generalTasks.front());
                generalTasks.pop();
            }
            task();
        }
    }

    void chunkWorker() {
        const size_t maxTasksPerFrame = 3;

        while (true) {
            size_t tasksDone = 0;

            while (tasksDone < maxTasksPerFrame) {
                PriorityTask ptask;
                {
                    std::unique_lock lock(chunkMutex);
                    if (chunkTasks.empty()) break;

                    std::sort(chunkTasks.begin(), chunkTasks.end(),
                        [](const PriorityTask& a, const PriorityTask& b) {
                            return static_cast<int>(a.priority) > static_cast<int>(b.priority);
                        });

                    ptask = chunkTasks.front();
                    chunkTasks.erase(chunkTasks.begin());
                }
                ptask.task();
                ++tasksDone;
            }
            {
                std::unique_lock lock(chunkMutex);
                if (stopping && chunkTasks.empty()) return;
                if (chunkTasks.empty()) {
                    chunkCondition.wait(lock, [this] { return stopping || !chunkTasks.empty(); });
                    if (stopping && chunkTasks.empty()) return;
                }
            }
        }
    }

    std::vector<std::thread> workers;

    std::queue<std::function<void()>> generalTasks;
    std::vector<PriorityTask> chunkTasks;

    const size_t maxChunkTasks = 90;

    std::mutex generalMutex;
    std::condition_variable generalCondition;

    std::mutex chunkMutex;
    std::condition_variable chunkCondition;

    std::atomic<bool> stopping;
};
