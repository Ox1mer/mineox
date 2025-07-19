#pragma once
#include <GLFW/glfw3.h>

class AppState {
public:
    virtual void onEnter() = 0;
    virtual void onExit() = 0;
    virtual void update(float deltaTime) = 0;
    virtual void render() = 0;
    virtual void handleInput(GLFWwindow* window) = 0;
    virtual ~AppState() = default;
};
