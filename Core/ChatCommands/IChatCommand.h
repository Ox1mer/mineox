#pragma once

#include <string>

class IChatCommand {
public:
    virtual ~IChatCommand() = default;

    // args — всё что после команды
    virtual void execute(const std::string& args) = 0;
};