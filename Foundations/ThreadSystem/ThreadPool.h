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
    auto enqueueChunkTask(F&& f, Args&&... args)
        -> std::future<typename std::invoke_result_t<F, Args...>>
    {
        using ReturnType = typename std::invoke_result_t<F, Args...>;
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        std::future<ReturnType> result = task->get_future();

        {
            std::unique_lock lock(chunkMutex);
            if (stopping) {
                throw std::runtime_error("enqueueChunkTask on stopped ThreadPool");
            }
            chunkTasks.emplace([task]() { (*task)(); });
        }
        chunkCondition.notify_one();
        return result;
    }

private:
    ThreadPool() : stopping(false) {
        unsigned int cpu = std::thread::hardware_concurrency();
        if (cpu == 0) cpu = 2;
        size_t total = 10;
        size_t generalCount = total - 1;  // 1 thread reserved for chunk tasks

        workers.emplace_back([this] { chunkWorker(); });

        for (size_t i = 0; i < generalCount; ++i) {
            workers.emplace_back([this] { generalWorker(); });
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

    // main worker for general tasks
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
        const size_t maxTasksPerFrame = 2; // limit tasks for one cycle

        while (true) {
            size_t tasksDone = 0;

            while (tasksDone < maxTasksPerFrame) {
                std::function<void()> task;
                {
                    std::unique_lock lock(chunkMutex);
                    if (chunkTasks.empty()) break;
                    task = std::move(chunkTasks.front());
                    chunkTasks.pop();
                }
                task();
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
    std::queue<std::function<void()>> chunkTasks;

    std::mutex generalMutex;
    std::condition_variable generalCondition;

    std::mutex chunkMutex;
    std::condition_variable chunkCondition;

    std::atomic<bool> stopping;
};