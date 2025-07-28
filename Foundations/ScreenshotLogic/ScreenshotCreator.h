#pragma once

#include "Logger.h"
#include <GLFW/glfw3.h>
#include <stb_image_write.h>

#include <windows.h>

class ScreenshotCreator {
public:
    static ScreenshotCreator& getInstance() {
        static ScreenshotCreator instance;
        return instance;
    }

    ScreenshotCreator() = default;
    ~ScreenshotCreator() = default;
    ScreenshotCreator(const ScreenshotCreator&) = delete;
    ScreenshotCreator& operator=(const ScreenshotCreator&) = delete;
    ScreenshotCreator(ScreenshotCreator&&) = delete;
    ScreenshotCreator& operator=(ScreenshotCreator&&) = delete;

    void doTheScreenshotAndSave(GLFWwindow* window) {
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