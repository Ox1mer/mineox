#pragma once
#include <filesystem>
#include <fstream>
#include <shared_mutex>
#include <string>
#include <stdexcept>
#include <mutex>
#include <vector>

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
    FileHandler() = default;
    ~FileHandler() = default;

    FileHandler(const FileHandler&) = delete;
    FileHandler& operator=(const FileHandler&) = delete;
    FileHandler(FileHandler&&) = delete;
    FileHandler& operator=(FileHandler&&) = delete;

    mutable std::shared_mutex fileMutex;
};