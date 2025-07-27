#pragma once

#include <filesystem>
#include <fstream>
#include <shared_mutex>
#include <string>
#include <stdexcept>
#include <mutex>
#include <vector>
#include <iostream>
#include <windows.h>
#include <set>

namespace fs = std::filesystem;

class FileHandler {
public:
    static FileHandler& getInstance() {
        static FileHandler instance;
        return instance;
    }

    void createDirectory(const fs::path& dirPath) {
        std::unique_lock lock(fileMutex);
        if (!fs::exists(dirPath)) {
            fs::create_directories(dirPath);
        }
    }

    void deleteDirectory(const fs::path& dirPath) {
        std::unique_lock lock(fileMutex);
        if (fs::exists(dirPath)) {
            fs::remove_all(dirPath);
        }
    }

    void copyFile(const fs::path& source, const fs::path& destination) {
        std::unique_lock lock(fileMutex);
        if (fs::exists(source)) {
            fs::copy(source, destination, fs::copy_options::overwrite_existing);
        }
    }

    void moveFile(const fs::path& source, const fs::path& destination) {
        std::unique_lock lock(fileMutex);
        if (fs::exists(source)) {
            fs::rename(source, destination);
        }
    }

    bool fileExists(const fs::path& filePath) const {
        std::shared_lock lock(fileMutex);
        return fs::exists(filePath);
    }

    void createFile(const fs::path& filePath) {
        std::unique_lock lock(fileMutex);
        std::ofstream file(filePath);
        if (!file) {
            throw std::runtime_error("Failed to create file: " + filePath.string());
        }
    }

    void deleteFile(const fs::path& filePath) {
        std::unique_lock lock(fileMutex);
        if (fs::exists(filePath)) {
            fs::remove(filePath);
        }
    }

    void writeToFile(const fs::path& filePath, const std::string& content, std::ios_base::openmode mode = std::ios::out | std::ios::app) {
        std::unique_lock lock(fileMutex);
        std::ofstream file(filePath, mode);
        if (!file) {
            throw std::runtime_error("Failed to open file for writing: " + filePath.string());
        }
        file << content;
    }

    void readFromFile(const fs::path& filePath, std::string& content) {
        std::shared_lock lock(fileMutex);
        std::ifstream file(filePath);
        if (!file) {
            throw std::runtime_error("Failed to open file for reading: " + filePath.string());
        }
        content.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    }

    void clearFile(const fs::path& filePath) {
        std::unique_lock lock(fileMutex);
        std::ofstream file(filePath, std::ios::trunc);
        if (!file) {
            throw std::runtime_error("Failed to clear file: " + filePath.string());
        }
    }

private:
    FileHandler() {
        char buffer[MAX_PATH];
        GetModuleFileNameA(NULL, buffer, MAX_PATH);
        fs::path exePath(buffer);
        fs::path exeDir = exePath.parent_path();
        fs::path dataPath = exeDir / "data";

        if (!fs::exists(dataPath)) {
            std::runtime_error("Data directory does not exist: " + dataPath.string() + ". Check if the application is running in the correct directory. If not, please download the data folder from the repository.");
        }
    };
    ~FileHandler() = default;

    void ensureFontPresent(const fs::path& fontPath) {
        if (!fs::exists(fontPath)) {
            std::cout << "Arial.ttf not found in target folder. Copying from data/..." << std::endl;

            char buffer[MAX_PATH];
            GetModuleFileNameA(NULL, buffer, MAX_PATH);
            fs::path exePath(buffer);
            fs::path exeDir = exePath.parent_path();
            fs::path sourceFont = exeDir / "data" / "fonts" / "arial.ttf";

            if (!fs::exists(sourceFont)) {
                throw std::runtime_error("Source font not found in data/ folder: " + sourceFont.string());
            }

            fs::copy_file(sourceFont, fontPath, fs::copy_options::overwrite_existing);

            std::cout << "Arial.ttf copied successfully to: " << fontPath.string() << std::endl;
        } else {
            std::cout << "Arial.ttf already exists at: " << fontPath.string() << std::endl;
        }
    }
    friend class PathProvider;

    void ensureShaderPresent(const fs::path& shaderPath) {
        char buffer[MAX_PATH];
        GetModuleFileNameA(NULL, buffer, MAX_PATH);
        fs::path exePath(buffer);
        fs::path exeDir = exePath.parent_path();

        fs::path sourceShaders = exeDir / "data" / "shaders";

        if (!fs::exists(sourceShaders) || !fs::is_directory(sourceShaders)) {
            throw std::runtime_error("Source shaders folder not found: " + sourceShaders.string());
        }

        if (!fs::exists(shaderPath)) {
            fs::create_directories(shaderPath);
        }

        std::set<fs::path> sourceFiles;
        for (const auto& entry : fs::directory_iterator(sourceShaders)) {
            if (entry.is_regular_file()) {
                sourceFiles.insert(entry.path().filename());
            }
        }

        for (const auto& file : sourceFiles) {
            fs::path targetFile = shaderPath / file;
            fs::path sourceFile = sourceShaders / file;

            if (!fs::exists(targetFile)) {
                std::cout << "Copying shader: " << file << std::endl;
                fs::copy_file(sourceFile, targetFile, fs::copy_options::overwrite_existing);
            }
        }

        for (const auto& entry : fs::directory_iterator(shaderPath)) {
            if (entry.is_regular_file()) {
                auto filename = entry.path().filename();
                if (sourceFiles.find(filename) == sourceFiles.end()) {
                    std::cout << "Deleting obsolete shader: " << filename << std::endl;
                    fs::remove(entry.path());
                }
            }
        }

        std::cout << "Shaders synced to: " << shaderPath << std::endl;
    }
    friend class PathProvider;

    void ensureTexturesPresent(const fs::path& texturesPath) {
        char buffer[MAX_PATH];
        GetModuleFileNameA(NULL, buffer, MAX_PATH);
        fs::path exePath(buffer);
        fs::path exeDir = exePath.parent_path();

        fs::path sourceTextures = exeDir / "data" / "textures";

        syncDirectories(sourceTextures, texturesPath);

        std::cout << "Textures synced to: " << texturesPath << std::endl;
    }
    friend class PathProvider;

    FileHandler(const FileHandler&) = delete;
    FileHandler& operator=(const FileHandler&) = delete;
    FileHandler(FileHandler&&) = delete;
    FileHandler& operator=(FileHandler&&) = delete;

    void syncDirectories(const fs::path& sourceDir, const fs::path& targetDir) {
        if (!fs::exists(sourceDir) || !fs::is_directory(sourceDir)) {
            throw std::runtime_error("Source directory not found or is not a directory: " + sourceDir.string());
        }

        if (!fs::exists(targetDir)) {
            fs::create_directories(targetDir);
        }

        std::set<fs::path> sourceEntries;
        for (const auto& entry : fs::directory_iterator(sourceDir)) {
            sourceEntries.insert(entry.path().filename());
        }

        for (const auto& name : sourceEntries) {
            fs::path sourcePath = sourceDir / name;
            fs::path targetPath = targetDir / name;

            if (fs::is_directory(sourcePath)) {
                syncDirectories(sourcePath, targetPath);
            } else if (fs::is_regular_file(sourcePath)) {
                if (!fs::exists(targetPath) || fs::file_size(sourcePath) != fs::file_size(targetPath)) {
                    std::cout << "Copying file: " << sourcePath << " -> " << targetPath << std::endl;
                    fs::copy_file(sourcePath, targetPath, fs::copy_options::overwrite_existing);
                }
            }
        }

        for (const auto& entry : fs::directory_iterator(targetDir)) {
            fs::path name = entry.path().filename();
            if (sourceEntries.find(name) == sourceEntries.end()) {
                std::cout << "Removing obsolete: " << entry.path() << std::endl;
                fs::remove_all(entry.path());
            }
        }
    }

    mutable std::shared_mutex fileMutex;
};