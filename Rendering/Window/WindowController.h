#pragma once
#include <string>
#include "Logger.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Camera.h"

class WindowController {
public:
    void init() {
        if (!glfwInit()) {
            Logger::getInstance().Log("Failed to initialize GLFW", LogLevel::Error, LogOutput::Both, LogWriteMode::Append);
            return;
        }
        createWindow();
        if (!window) {
            Logger::getInstance().Log("Failed to create GLFW window", LogLevel::Error, LogOutput::Both, LogWriteMode::Append);
            glfwTerminate();
        } else {
            Logger::getInstance().Log("GLFW window created successfully", LogLevel::Info, LogOutput::Both, LogWriteMode::Append);
        }
    };

    void shutdown() {
        destroyWindow();
        glfwTerminate();
        Logger::getInstance().Log("GLFW terminated successfully", LogLevel::Info, LogOutput::Both, LogWriteMode::Append);
    };

    void toggleFullscreen() {
        fullscreen = !fullscreen;
        destroyWindow();
        createWindow();
        Logger::getInstance().Log(fullscreen ? "Switched to fullscreen mode" : "Switched to windowed mode", LogLevel::Info, LogOutput::Both, LogWriteMode::Append);
    };

    void setResolution(int width, int height) {
        this->width = width;
        this->height = height;
        destroyWindow();
        createWindow();
        Logger::getInstance().Log("Resolution set to " + std::to_string(width) + "x" + std::to_string(height), LogLevel::Info, LogOutput::Both, LogWriteMode::Append);
    };

    void setVSync(bool enabled) {
        vsyncEnabled = enabled;
        if (window) {
            glfwSwapInterval(vsyncEnabled ? 1 : 0);
            Logger::getInstance().Log(vsyncEnabled ? "VSync enabled" : "VSync disabled", LogLevel::Info, LogOutput::Both, LogWriteMode::Append);
        }
    }

    void setWindowTitle(const std::string& title) {
        if (window) {
            glfwSetWindowTitle(window, title.c_str());
            Logger::getInstance().Log("Window title set to: " + title, LogLevel::Info, LogOutput::Both, LogWriteMode::Append);
        } else {
            Logger::getInstance().Log("Failed to set window title: Window not initialized", LogLevel::Error, LogOutput::Both, LogWriteMode::Append);
        }
    }

    const float getWidth() const {
        return width;
    }

    const float getHeight() const {
        return height;
    }

    GLFWwindow* getWindow() const {
        return window;
    }

    void bindCameraControls(Camera* camera) {
        if (!window) {
            Logger::getInstance().Log("Cannot bind camera controls: Window not initialized", LogLevel::Error, LogOutput::Both, LogWriteMode::Append);
            return;
        }

        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        glfwSetWindowUserPointer(window, camera);

        glfwSetCursorPosCallback(window, [](GLFWwindow* win, double xpos, double ypos) {
            auto cam = static_cast<Camera*>(glfwGetWindowUserPointer(win));

            static float lastX = 0.0f;
            static float lastY = 0.0f;
            static bool firstMouse = true;

            if (firstMouse) {
                lastX = xpos;
                lastY = ypos;
                firstMouse = false;
            }

            float xoffset = xpos - lastX;
            float yoffset = lastY - ypos;

            lastX = xpos;
            lastY = ypos;

            cam->ProcessMouseMovement(xoffset, yoffset);
        });

        Logger::getInstance().Log("Camera controls bound (mouse look active)", LogLevel::Info, LogOutput::Both, LogWriteMode::Append);
    }


    void processCameraKeyboard(Camera* camera, float deltaTime) {
        if (!window) {
            Logger::getInstance().Log("Cannot process camera keyboard: Window not initialized", LogLevel::Error, LogOutput::Both, LogWriteMode::Append);
            return;
        }

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera->ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera->ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera->ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera->ProcessKeyboard(RIGHT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            camera->ProcessKeyboard(UP, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            camera->ProcessKeyboard(DOWN, deltaTime);
    }


private:
    GLFWwindow* window = nullptr;
    int width = 1280;
    int height = 720;
    bool fullscreen = false;
    bool vsyncEnabled = false;

    void createWindow() {
        if (fullscreen) {
            const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
            width = mode->width;
            height = mode->height;
            window = glfwCreateWindow(width, height, "MineOx", glfwGetPrimaryMonitor(), nullptr);
        } else {
            window = glfwCreateWindow(width, height, "MineOx", nullptr, nullptr);
        }

        if (!window) {
            Logger::getInstance().Log("Failed to create GLFW window", LogLevel::Error, LogOutput::Both, LogWriteMode::Append);
            return;
        }

        glfwMakeContextCurrent(window);
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            Logger::getInstance().Log("Failed to initialize GLAD", LogLevel::Error, LogOutput::Both, LogWriteMode::Append);
            return;
        }

        if (vsyncEnabled) {
            glfwSwapInterval(1);
        } else {
            glfwSwapInterval(0);
        }

        updateWindowTitle();
    };

    void destroyWindow() {
        if (window) {
            glfwDestroyWindow(window);
            window = nullptr;
            Logger::getInstance().Log("GLFW window destroyed", LogLevel::Info, LogOutput::Both, LogWriteMode::Append);
        } else {
            Logger::getInstance().Log("No GLFW window to destroy", LogLevel::Warning, LogOutput::Both, LogWriteMode::Append);
        }
    };

    void updateWindowTitle() {
        if (window) {
            std::string title = "MineOx - " + std::to_string(width) + "x" + std::to_string(height);
            glfwSetWindowTitle(window, title.c_str());
            Logger::getInstance().Log("Window title updated to: " + title, LogLevel::Info, LogOutput::Both, LogWriteMode::Append);
        } else {
            Logger::getInstance().Log("Failed to update window title: Window not initialized", LogLevel::Error, LogOutput::Both, LogWriteMode::Append);
        }
    };

};