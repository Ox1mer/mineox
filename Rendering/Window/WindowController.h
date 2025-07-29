#pragma once
#include <string>
#include "Logger.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Camera.h"
#include "ServiceLocator.h"
#include "Blocks.h"
#include "RayCastHit.h"
#include "BlockPos.h"
#include <optional>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <stb_image_write.h>

#include <chrono>
#include <iomanip>
#include <sstream>

#include <windows.h>

#include "ChatController.h"
#include "WindowContext.h"

namespace fs = std::filesystem;

class WindowController {
public:
    bool shouldToggleFullscreen = false;
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
        shouldToggleFullscreen = false;
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
            auto* ctx = static_cast<WindowContext*>(glfwGetWindowUserPointer(win));
            auto* chat = ctx->chat;

            if (chat && !chat->getVisibility()) {
                auto* ctx = static_cast<WindowContext*>(glfwGetWindowUserPointer(win));
                auto* cam = ctx->camera;

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
            }
        });

        Logger::getInstance().Log("Camera controls bound (mouse look active)", LogLevel::Info, LogOutput::Both, LogWriteMode::Append);
    }

    void registerCharCallback() {
        glfwSetCharCallback(window, [](GLFWwindow* window, unsigned int codepoint) {
            auto* ctx = static_cast<WindowContext*>(glfwGetWindowUserPointer(window));
            auto* chat = ctx->chat;

            if (chat && chat->getVisibility()) {
                chat->addSymbol(chat->utf32ToUtf8(codepoint));
            }
        });
    }

private:
    GLFWwindow* window = nullptr;
    int width = 1280;
    int height = 720;
    bool fullscreen = false;
    bool vsyncEnabled = false;

    void createWindow() {
        if (fullscreen) {
            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode* mode = glfwGetVideoMode(monitor);

            width = mode->width;
            height = mode->height;

            window = glfwCreateWindow(width, height, "MineOx", monitor, nullptr);
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