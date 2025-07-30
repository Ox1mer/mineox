#pragma once

#include "IChatCommand.h"
#include "ChatController.h"
#include "ServiceLocator.h"
#include <sstream>
#include "glm/glm.hpp"
#include "Blocks.h"

class ChooseBlockCommand : public IChatCommand {
public:
    ChooseBlockCommand(ChatController& controller) : _controller(controller) {}

    void execute(const std::string& args) override {
        std::istringstream iss(args);
        int blockId;

        if (!(iss >> blockId)) {
            _controller.addMessage("Usage: /chblock <blockid>");
            return;
        }

        ServiceLocator::getCamera()->choosedBlock = static_cast<Blocks>(blockId);

        _controller.addMessage("Succesfully choosed the block " + std::to_string(blockId));
    }

private:
    ChatController& _controller;
};