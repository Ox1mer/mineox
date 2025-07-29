#pragma once

#include "IChatCommand.h"
#include "ChatController.h"
#include "ServiceLocator.h"
#include <sstream>
#include "glm/glm.hpp"

class TpCommand : public IChatCommand {
public:
    TpCommand(ChatController& controller) : _controller(controller) {}

    void execute(const std::string& args) override {
        std::istringstream iss(args);
        float x, y, z;

        if (!(iss >> x >> y >> z)) {
            _controller.addMessage("Usage: /tp <x> <y> <z>");
            return;
        }

        auto camera = ServiceLocator::getCamera();
        camera->Position = glm::vec3(x, y, z);

        _controller.addMessage("Teleported to: " + std::to_string(x) + " " +
                            std::to_string(y) + " " +
                            std::to_string(z));
    }

private:
    ChatController& _controller;
};