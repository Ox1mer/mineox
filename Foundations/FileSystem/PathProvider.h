#pragma once

#include <filesystem>
#include <stdexcept>
#include <array>
#include "ChunkPos.h"

#include "FileHandler.h"

namespace fs = std::filesystem;

class PathProvider {
public:
    static PathProvider& getInstance() {
        static PathProvider instance;
        return instance;
    }

    fs::path getRootPath() const {
        return rootPath;
    }

    fs::path getConfigPath() const {
        return configPath;
    }

    fs::path getLogsPath() const {
        return logsPath;
    }

    fs::path getDataPath() const {
        return dataPath;
    }

    fs::path getWorldsPath() const {
        return worldsPath;
    }

    fs::path getTextureFolderPath() {
        return textureFolderPath;
    }

    fs::path getBlocksTextureFolderPath() {
        return textureFolderPath / "blocks";
    }

    fs::path getBlockTexturePath(std::string blockStringRepresentation) {
        return getBlocksTextureFolderPath() / (blockStringRepresentation + ".png");
    }

    fs::path getWorldChunksPath(std::string worldName) const {
        return worldsPath / worldName / "chunks";
    }

    fs::path getChunkFilePath(std::string worldName, ChunkPos chunkPos) const {
        return getWorldChunksPath(worldName) / chunkPos.toString();
    }

    fs::path getFontsPath() const {
        return dataPath / "fonts";
    }

    fs::path getShadersPath() const {
        return dataPath / "shaders";
    }

    fs::path getScreenshotsPath() const {
        return screenshotsFolderPath;
    }

    // -- Shaders paths
    std::array<fs::path, 2> getWireFrameCubeShadersPath() const {
        return {
            dataPath / "shaders" / "wireFrameCubeVertexShader.vs",
            dataPath / "shaders" / "wireFrameCubeFragmentShader.fs"
        };
    }

    std::array<fs::path, 2> getFontShadersPath() const {
        return {
            dataPath / "shaders" / "fontVertShader.vs",
            dataPath / "shaders" / "fontFragmentShader.fs"
        };
    }

    std::array<fs::path, 2> getMainShadersPath() const {
        return {
            dataPath / "shaders" / "vertexShader.vs",
            dataPath / "shaders" / "fragmentShader.fs"
        };
    }

    std::array<fs::path, 2> getDepthShaderPath() const {
        return {
            dataPath / "shaders" / "depth.vs",
            dataPath / "shaders" / "depth.fs"
        };
    }

    std::array<fs::path, 2> getChatShadersPath() const {
        return {
            dataPath / "shaders" / "chat.vs",
            dataPath / "shaders" / "chat.fs"
        };
    }

private:

    void createDirectories() {
        auto& fh = FileHandler::getInstance();
        fh.createDirectory(configPath);
        fh.createDirectory(screenshotsFolderPath);
        fh.createDirectory(logsPath);
        fh.createDirectory(dataPath);
        fh.createDirectory(worldsPath);
        fh.createDirectory(textureFolderPath);
        fh.createDirectory(getFontsPath());
        fh.createDirectory(getShadersPath());
        fh.createDirectory(getBlocksTextureFolderPath());
        fh.ensureFontPresent(getFontsPath() / "arial.ttf");
        fh.ensureShaderPresent(getShadersPath());
        fh.ensureTexturesPresent(getTextureFolderPath());
    }

    PathProvider() {
        const char* appdata = std::getenv("APPDATA");
        if (!appdata) throw std::runtime_error("APPDATA env var not found");
        rootPath = fs::path(appdata) / ".mineox";
        configPath = rootPath / "config";
        logsPath = rootPath / "logs";
        dataPath = rootPath / "data";
        worldsPath = rootPath / "saves";
        textureFolderPath = dataPath / "textures";
        screenshotsFolderPath = rootPath / "screenshots";

        createDirectories();
    }

    ~PathProvider() = default;

    fs::path rootPath;
    fs::path configPath;
    fs::path logsPath;
    fs::path dataPath;
    fs::path worldsPath;
    fs::path textureFolderPath;
    fs::path screenshotsFolderPath;

    PathProvider(const PathProvider&) = delete;
    PathProvider& operator=(const PathProvider&) = delete;
    PathProvider(PathProvider&&) = delete;
    PathProvider& operator=(PathProvider&&) = delete;
};