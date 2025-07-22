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

namespace fs = std::filesystem;

class WindowController {
private:
    inline static float leftMouseCooldown = 0.0f;
    inline static float rightMouseCooldown = 0.0f;
    const float cooldownTime = 0.2f;

    bool leftMousePressedLastFrame = false;
    bool rightMousePressedLastFrame = false;

public:
    bool shouldTakeScreenshot = false;
    bool shouldToggleFullscreen = false;
    void init() {
        leftMouseCooldown, rightMouseCooldown = 0.0f;
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


    void processCameraKeyboard(Camera* camera, float deltaTime, std::optional<RaycastHit>& raycastHit, Blocks& choosedBlock) {
        if (!window) {
            Logger::getInstance().Log("Cannot process camera keyboard: Window not initialized", LogLevel::Error, LogOutput::Both, LogWriteMode::Append);
            return;
        }

        if (leftMouseCooldown > 0.0f)
            leftMouseCooldown -= deltaTime;
        if (rightMouseCooldown > 0.0f)
            rightMouseCooldown -= deltaTime;

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
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS) {
            shouldTakeScreenshot = true;
        }

        bool leftPressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
        if (leftPressed && raycastHit) {
            if (!leftMousePressedLastFrame) {
                ServiceLocator::GetWorld()->getChunkController().breakBlock(BlockPos(raycastHit.value().blockPos));
                leftMouseCooldown = cooldownTime;
            } else if (leftMouseCooldown <= 0.0f) {
                ServiceLocator::GetWorld()->getChunkController().breakBlock(BlockPos(raycastHit.value().blockPos));
                leftMouseCooldown = cooldownTime;
            }
        }
        leftMousePressedLastFrame = leftPressed;

        bool rightPressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
        if (rightPressed && raycastHit) {
            if (!rightMousePressedLastFrame) {
                BlockPos newBlockPos(raycastHit.value().blockPos + raycastHit.value().faceNormal);
                ServiceLocator::GetWorld()->getChunkController().setBlock(newBlockPos, choosedBlock);
                rightMouseCooldown = cooldownTime;
            } else if (rightMouseCooldown <= 0.0f) {
                BlockPos newBlockPos(raycastHit.value().blockPos + raycastHit.value().faceNormal);
                ServiceLocator::GetWorld()->getChunkController().setBlock(newBlockPos, choosedBlock);
                rightMouseCooldown = cooldownTime;
            }
        }
        rightMousePressedLastFrame = rightPressed;
    }

    void doTheScreenshotAndSave() {
        int Lwidth, Lheight;
        glfwGetFramebufferSize(window, &Lwidth, &Lheight);

        std::vector<unsigned char> pixels(Lwidth * Lheight * 3);

        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadPixels(0, 0, Lwidth, Lheight, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

        std::vector<unsigned char> flippedPixels(Lwidth * Lheight * 3);
        for (int y = 0; y < Lheight; ++y) {
            memcpy(&flippedPixels[y * Lwidth * 3], &pixels[(Lheight - 1 - y) * Lwidth * 3], Lwidth * 3);
        }

        auto filename = getScreenshotFilename();
        auto folder = PathProvider::getInstance().getScreenshotsPath();
        if (!std::filesystem::exists(folder))
            std::filesystem::create_directories(folder);

        auto fullPath = folder / filename;

        stbi_write_png(fullPath.string().c_str(), Lwidth, Lheight, 3, flippedPixels.data(), Lwidth * 3);

        Logger::getInstance().Log("Screenshot saved to: " + fullPath.string(), LogLevel::Info, LogOutput::Both, LogWriteMode::Append);

        copyFramebufferToClipboard(flippedPixels, fullPath, Lwidth , Lheight);
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

    std::string getScreenshotFilename() {
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        std::tm localTime;
    #ifdef _WIN32
        localtime_s(&localTime, &now_c);
    #else
        localtime_r(&now_c, &localTime);
    #endif

        std::ostringstream oss;
        oss << "screenshot_"
            << std::put_time(&localTime, "%Y-%m-%d_%H-%M-%S")
            << ".png";
        return oss.str();
    }

    void copyFramebufferToClipboard(const std::vector<unsigned char> pixels, const fs::path& imagePath, int Lwidth, int Lheight) {
        BITMAPINFOHEADER bih = {};
        bih.biSize = sizeof(BITMAPINFOHEADER);
        bih.biWidth = Lwidth;
        bih.biHeight = -Lheight; // top-down
        bih.biPlanes = 1;
        bih.biBitCount = 32; // BGRA
        bih.biCompression = BI_RGB;
        bih.biSizeImage = Lwidth * Lheight * 4;

        std::vector<unsigned char> bgraPixels(Lwidth * Lheight * 4);
        for (int i = 0; i < Lwidth * Lheight; ++i) {
            bgraPixels[i * 4 + 0] = pixels[i * 3 + 2]; // B
            bgraPixels[i * 4 + 1] = pixels[i * 3 + 1]; // G
            bgraPixels[i * 4 + 2] = pixels[i * 3 + 0]; // R
            bgraPixels[i * 4 + 3] = 255;                // A
        }

        HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, sizeof(BITMAPINFOHEADER) + bgraPixels.size());
        if (!hGlobal) {
            // error
            return;
        }

        void* pData = GlobalLock(hGlobal);
        if (!pData) {
            GlobalFree(hGlobal);
            return;
        }

        memcpy(pData, &bih, sizeof(BITMAPINFOHEADER));
        memcpy((char*)pData + sizeof(BITMAPINFOHEADER), bgraPixels.data(), bgraPixels.size());
        GlobalUnlock(hGlobal);

        if (OpenClipboard(nullptr)) {
            EmptyClipboard();
            SetClipboardData(CF_DIB, hGlobal);
            CloseClipboard();
        } else {
            GlobalFree(hGlobal);
            // error log
            return;
        }

        Logger::getInstance().Log("Framebuffer copied to clipboard from: " + imagePath.string(), LogLevel::Info, LogOutput::Both, LogWriteMode::Append);
    }

};