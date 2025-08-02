#pragma once

#include <iostream>
#include <filesystem>
#include <vector>
#include <unordered_map>

#include <stb_image.h>
#include <stb_image_write.h>

#include "PathProvider.h"
#include "Logger.h"

namespace fs = std::filesystem;

struct AtlasRegion {
    float u0, v0; // Нижний левый угол
    float u1, v1; // Верхний правый угол
};

class TextureManager {
private:
    int textureSize = 16;
    int offset = 2;

    GLuint atlasTextureID = 0;
    std::unordered_map<std::string, AtlasRegion> atlasRegions;

    TextureManager() = default;

    std::pair<int, int> calculateAtlasSize(int n) {
        int w = static_cast<int>(std::floor(std::sqrt(n)));
        int h = static_cast<int>(std::ceil(static_cast<float>(n) / w));
        return {w, h};
    }

    void saveAtlasToFile(const std::string& filename, int w, int h) {
        glBindTexture(GL_TEXTURE_2D, atlasTextureID);

        std::vector<unsigned char> pixels(w * h * 4); // RGBA

        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

        stbi_write_png(filename.c_str(), w, h, 4, pixels.data(), w * 4);

        Logger::getInstance().Log("Saved atlas to: " + filename);
    }

public:
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    static TextureManager& getInstance() {
        static TextureManager instance;
        return instance;
    }

    GLuint& getAtlasID() { return atlasTextureID; }
    std::unordered_map<std::string, AtlasRegion>& getAtlasRegions() { return atlasRegions; }

    void generateEmptyTextureAtlas(int w, int h) {
        if (atlasTextureID) {
            glDeleteTextures(1, &atlasTextureID);
        }

        glGenTextures(1, &atlasTextureID);
        glBindTexture(GL_TEXTURE_2D, atlasTextureID);

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA,
            w,
            h,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            nullptr
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    void bindTextures() {
        auto blocksTexturesFolderPath = PathProvider::getInstance().getBlocksTextureFolderPath();
        int count = 0;

        std::vector<fs::path> textures;

        try {
            for (const auto& entry : fs::directory_iterator(blocksTexturesFolderPath)) {
                if (entry.is_regular_file() && entry.path().extension() == ".png") {
                    std::string fullPath = entry.path().string();

                    int width, height, channels;
                    unsigned char* data = stbi_load(fullPath.c_str(), &width, &height, &channels, 0);
                    if (data) {
                        if (width == 16 && height == 16) {
                            Logger::getInstance().Log(entry.path().filename().string() + " - размер подходит: " + std::to_string(width) + "x" + std::to_string(height));
                            textures.push_back(entry.path());
                            ++count;
                        } else {
                            Logger::getInstance().Log(entry.path().filename().string() + " - размер не подходит: " + std::to_string(width) + "x" + std::to_string(height), LogLevel::Warning);
                        }
                        stbi_image_free(data);
                    } else {
                        Logger::getInstance().Log(entry.path().filename().string() + " - не удалось загрузить", LogLevel::Error);
                    }
                }
            }
            Logger::getInstance().Log("Count of correct png files: " + std::to_string(count));
        } catch (const fs::filesystem_error& e) {
            Logger::getInstance().Log("Error", LogLevel::Error);
        }

        if (count > 0) {
            auto sizeOfTextureAtlas = calculateAtlasSize(count);
            Logger::getInstance().Log("Size of texture atlas: " + std::to_string(sizeOfTextureAtlas.first) + "x" + std::to_string(sizeOfTextureAtlas.second));
            std::pair<int, int> sizeOfTextureAtlasInPixels = {
                (sizeOfTextureAtlas.first * textureSize) + (sizeOfTextureAtlas.first * offset) + offset,
                (sizeOfTextureAtlas.second * textureSize) + (sizeOfTextureAtlas.second * offset) + offset
            };
            Logger::getInstance().Log("Size of texture atlas by pixels: " +
                                      std::to_string(sizeOfTextureAtlasInPixels.first) + "x" +
                                      std::to_string(sizeOfTextureAtlasInPixels.second));

            float atlasWidth = static_cast<float>(sizeOfTextureAtlasInPixels.first);
            float atlasHeight = static_cast<float>(sizeOfTextureAtlasInPixels.second);

            generateEmptyTextureAtlas(sizeOfTextureAtlasInPixels.first, sizeOfTextureAtlasInPixels.second);

            int index = 0;

            for (const auto& path : textures) {
                int width, height, channels;
                unsigned char* data = stbi_load(path.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);

                if (data) {
                    int xIndex = index % sizeOfTextureAtlas.first;
                    int yIndex = index / sizeOfTextureAtlas.first;

                    int xOffset = offset + xIndex * (textureSize + offset);
                    int yOffset = offset + yIndex * (textureSize + offset);

                    glTexSubImage2D(
                        GL_TEXTURE_2D,
                        0,
                        xOffset,
                        yOffset,
                        textureSize,
                        textureSize,
                        GL_RGBA,
                        GL_UNSIGNED_BYTE,
                        data
                    );

                    AtlasRegion region;
                    region.u0 = static_cast<float>(xOffset) / atlasWidth;
                    region.v0 = static_cast<float>(yOffset) / atlasHeight;
                    region.u1 = static_cast<float>(xOffset + textureSize) / atlasWidth;
                    region.v1 = static_cast<float>(yOffset + textureSize) / atlasHeight;

                    std::string fileName = path.filename().string();
                    atlasRegions[fileName] = region;

                    Logger::getInstance().Log("Added region for: " + fileName +
                                              " UV: (" + std::to_string(region.u0) + ", " +
                                              std::to_string(region.v0) + ") - (" +
                                              std::to_string(region.u1) + ", " +
                                              std::to_string(region.v1) + ")");

                    stbi_image_free(data);

                    saveAtlasToFile("atlas.png", static_cast<int>(atlasWidth), static_cast<int>(atlasHeight));

                } else {
                    Logger::getInstance().Log("Cant load: " + path.string());
                }
                ++index;
            }

            glGenerateMipmap(GL_TEXTURE_2D);
        }
    }
};