#pragma once

#include <chrono>
#include <string>
#include <mutex>

#include "Logger.h"

class ScopedTimer {
public:
    explicit ScopedTimer(const std::string& name)
        : m_name(name), m_start(std::chrono::high_resolution_clock::now())
    {}

    ~ScopedTimer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(end - m_start).count();

        Logger::getInstance().Log(m_name + " took " + std::to_string(durationMs) + " ms", LogLevel::ScopedTimer);
    }

    ScopedTimer(const ScopedTimer&) = delete;
    ScopedTimer& operator=(const ScopedTimer&) = delete;
    ScopedTimer(ScopedTimer&&) = delete;
    ScopedTimer& operator=(ScopedTimer&&) = delete;

private:
    std::string m_name;
    std::chrono::high_resolution_clock::time_point m_start;
};
