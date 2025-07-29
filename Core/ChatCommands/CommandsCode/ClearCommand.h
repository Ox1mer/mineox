#pragma once

#include "IChatCommand.h"
#include "ChatController.h"
#include <sstream>

class ClearCommand : public IChatCommand {
public:
    ClearCommand(ChatController& controller) : _controller(controller) {}

    void execute(const std::string& args) override {
        _controller.clearMessages();
        _controller.addMessage("Чат очищен.");
    }

private:
    ChatController& _controller;
};