#pragma once
#include <memory>
#include <GLFW/glfw3.h>
#include "AppState.h"

class StateController {
public:
    void changeState(std::unique_ptr<AppState> newState) {
        if (current) current->onExit();
        current = std::move(newState);
        if (current) current->onEnter();
    }

    void update(float dt) {
        if (current) current->update(dt);
    }

    void render() {
        if (current) current->render();
    }

    void handleInput(GLFWwindow* window) {
        if (current) current->handleInput(window);
    }

private:
    std::unique_ptr<AppState> current;
};
