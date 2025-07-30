#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <iostream>
#include <algorithm>
#include <glad/glad.h>
#include <stb_image.h>

#include "Logger.h"
#include "Blocks.h"

namespace fs = std::filesystem;

class TextureController {
public:
    static TextureController& getInstance() {
        static TextureController instance;
        return instance;
    }

    TextureController(const TextureController&) = delete;
    TextureController(TextureController&&) = delete;
    TextureController& operator=(const TextureController&) = delete;
    TextureController& operator=(TextureController&&) = delete;

    void initialize(const fs::path& folderPath, const std::vector<Blocks>& blocksToLoad) {
        textures.clear();
        
        GLuint dummyID = createDummyTexture();
        textures.emplace_back("Dummy", dummyID);
        Logger::getInstance().Log("Inserted dummy texture at index 0, ID: " + std::to_string(dummyID));

        for (Blocks block : blocksToLoad) {
            std::string name = toString(block);
            fs::path filePath = folderPath / (name + ".png");

            if (!fs::exists(filePath)) {
                std::cerr << "Texture file not found: " << filePath << std::endl;
                continue;
            }

            GLuint texID = loadTexture(filePath);
            if (texID != 0) {
                textures.emplace_back(name, texID);
                Logger::getInstance().Log("Succesfully emplace_back " + name + " " + std::to_string(texID));
            } else {
                std::cerr << "Failed to load texture: " << filePath << std::endl;
            }
        }
    }

    void initializeTextures(GLuint shaderProgramID, const std::vector<Blocks>& blocksOrder) {
        for (GLuint i = 0; i < blocksOrder.size(); ++i) {
            GLuint texID = getTextureID(blocksOrder[i]);
            glActiveTexture(GL_TEXTURE0 + i + 1);
            glBindTexture(GL_TEXTURE_2D, texID);
        }

        GLint location = glGetUniformLocation(shaderProgramID, "textures");
        if (location == -1) {
            Logger::getInstance().Log("Warning: uniform 'textures' not found in shader", LogLevel::Warning, LogOutput::Both);
            return;
        }

        std::vector<GLint> samplers(blocksOrder.size());
        for (int i = 0; i < samplers.size(); ++i)
            samplers[i] = i;

        glUseProgram(shaderProgramID);
        glUniform1iv(location, samplers.size(), samplers.data());
        glUseProgram(0);
    }

    GLuint getTextureID(const std::string& blockName) const {
        auto it = std::find_if(textures.begin(), textures.end(),
            [&](const auto& pair) {
                return pair.first == blockName;
            });
        if (it != textures.end()) {
            return it->second;
        }
        return textures[0].second; // Заглушка
    }

    GLuint getTextureID(Blocks block) const {
        return getTextureID(toString(block));
    }

    GLuint getTextureIDByIndex(size_t index) const {
        if (index >= textures.size()) throw std::out_of_range("Invalid texture index");
        return textures[index].second;
    }

    size_t getTextureCount() const {
        return textures.size();
    }

private:
    TextureController() = default;
    ~TextureController() = default;

    std::vector<std::pair<std::string, GLuint>> textures;

    GLuint loadTexture(const fs::path& path) const {
        int width, height, nrChannels;
        stbi_set_flip_vertically_on_load(true);
        unsigned char* data = stbi_load(path.string().c_str(), &width, &height, &nrChannels, 0);
        if (!data) return 0;

        GLenum format = (nrChannels == 1 ? GL_RED : (nrChannels == 3 ? GL_RGB : GL_RGBA));
        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);

        Logger::getInstance().Log("Loaded texture: " + path.string() + " ID: " + std::to_string(textureID));

        return textureID;
    }

    GLuint createDummyTexture() const {
        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        unsigned char pixel[4] = { 255, 0, 255, 255 };
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        return textureID;
    }
};