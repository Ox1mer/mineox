#pragma once
#include <filesystem>
#include <stdexcept>
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

private:
    PathProvider() {
        const char* appdata = std::getenv("APPDATA");
        if (!appdata) throw std::runtime_error("APPDATA env var not found");
        rootPath = fs::path(appdata) / ".mineox";
        configPath = rootPath / "config";
        logsPath = rootPath / "logs";
        dataPath = rootPath / "data";
        worldsPath = rootPath / "saves";
        textureFolderPath = dataPath / "textures";

        FileHandler::getInstance().createDirectory(configPath);
        FileHandler::getInstance().createDirectory(logsPath);
        FileHandler::getInstance().createDirectory(dataPath);
        FileHandler::getInstance().createDirectory(worldsPath);
        FileHandler::getInstance().createDirectory(textureFolderPath);
    }
    ~PathProvider() = default;

    fs::path rootPath;
    fs::path configPath;
    fs::path logsPath;
    fs::path dataPath;
    fs::path worldsPath;
    fs::path textureFolderPath;

    PathProvider(const PathProvider&) = delete;
    PathProvider& operator=(const PathProvider&) = delete;
    PathProvider(PathProvider&&) = delete;
    PathProvider& operator=(PathProvider&&) = delete;
};