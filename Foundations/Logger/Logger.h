#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include <memory>
#include <iomanip>
#include <mutex>
#include <atomic>
#include <sstream>
#include <filesystem>

#include "LogStrategy.h"
#include "PathProvider.h"
#include "FileHandler.h"

namespace fs = std::filesystem;

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error,
    Critical,
    ScopedTimer
};

enum class LogOutput {
    Console,
    File,
    Both
};

enum class LogWriteMode {
    Append,
    Overwrite
};

class Logger {
public:
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    void Log(const std::string& message,
             LogLevel logLevel = LogLevel::Info,
             LogOutput logOutput = LogOutput::Both,
             LogWriteMode logWriteMode = LogWriteMode::Append)
    {
        LogLevel lastLevel = lastLogLevel.load(std::memory_order_relaxed);
        if (logLevel != lastLevel) {
            switchLogStrategy(logLevel);
            lastLogLevel.store(logLevel, std::memory_order_relaxed);
        }

        auto strategy = currentStrategy.load(std::memory_order_acquire);
        if (!strategy) {
            std::cerr << "[Logger Error] No current logging strategy set.\n";
            return;
        }

        std::string timestamp = getCurrentTime();

        std::string consoleMessage = "[" + timestamp + "] " + strategy->getColor() + message + "\033[0m";
        std::string fileMessage = "[" + timestamp + "] " + message;

        std::lock_guard<std::mutex> lock(logMutex);

        if (logOutput == LogOutput::Console || logOutput == LogOutput::Both) {
            std::cout << consoleMessage << std::endl;
        }

        if ((logOutput == LogOutput::File || logOutput == LogOutput::Both)) {
            if (!logFile.is_open() || currentWriteMode != logWriteMode) {
                if (logFile.is_open()) {
                    logFile.close();
                }
                std::ios_base::openmode mode = (logWriteMode == LogWriteMode::Append) ? std::ios::app : std::ios::out;
                logFile.open(logFilePath, mode);
                if (!logFile.is_open()) {
                    std::cerr << "[Logger Error] Unable to open log file: " << logFilePath << std::endl;
                    return;
                }
                currentWriteMode = logWriteMode;
            }
            logFile << fileMessage << std::endl;
            logFile.flush();
        }
    }

private:
    Logger()
    {
        PathProvider::getInstance();
        switchLogStrategy(LogLevel::Info);

        auto& paths = PathProvider::getInstance();
        logFilePath = paths.getLogsPath() / "log.txt";

        FileHandler::getInstance().createDirectory(paths.getLogsPath());
    }

    ~Logger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    std::string getCurrentTime() {
        std::time_t now = std::time(nullptr);

        std::tm localTime;
#if defined(_MSC_VER) || defined(__MINGW32__)
        localtime_s(&localTime, &now);
#else
        localtime_r(&now, &localTime);
#endif

        std::ostringstream oss;
        oss << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

    std::shared_ptr<LogStrategy> makeStrategy(LogLevel level) {
        switch (level) {
            case LogLevel::Debug:    return std::make_shared<DebugStrategy>();
            case LogLevel::Info:     return std::make_shared<InfoStrategy>();
            case LogLevel::Warning:  return std::make_shared<WarningStrategy>();
            case LogLevel::Error:    return std::make_shared<ErrorStrategy>();
            case LogLevel::Critical: return std::make_shared<CriticalStrategy>();
            case LogLevel::ScopedTimer: return std::make_shared<ScopedTimerStrategy>();
            default:                 return std::make_shared<InfoStrategy>();
        }
    }

    void switchLogStrategy(LogLevel level) {
        currentStrategy.store(makeStrategy(level), std::memory_order_release);
    }

    std::mutex logMutex;
    std::ofstream logFile;
    fs::path logFilePath;
    LogWriteMode currentWriteMode = LogWriteMode::Append;

    std::atomic<std::shared_ptr<LogStrategy>> currentStrategy;
    std::atomic<LogLevel> lastLogLevel{LogLevel::Info};

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;
};
