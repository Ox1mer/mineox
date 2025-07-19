#pragma once
#include <string>
#include <iostream>

class LogStrategy {
public:
    virtual std::string getColor() const = 0;
    virtual ~LogStrategy() = default;
};

class DebugStrategy : public LogStrategy {
public:
    std::string getColor() const override { return "\033[37m"; }
};

class InfoStrategy : public LogStrategy {
public:
    std::string getColor() const override { return "\033[32m"; }
};

class WarningStrategy : public LogStrategy {
public: 
    std::string getColor() const override { return "\033[33m"; }
};

class ErrorStrategy : public LogStrategy {
public:
    std::string getColor() const override { return "\033[31m"; }
};

class CriticalStrategy : public LogStrategy {
public:
    std::string getColor() const override { return "\033[35m"; }
};

class ScopedTimerStrategy : public LogStrategy {
public:
    std::string getColor() const override { return "\033[95m"; }
};