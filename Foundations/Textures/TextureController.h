#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <filesystem>
#include <iostream>
#include <glad/glad.h>
#include <stb_image.h>

#include "Logger.h"

namespace fs = std::filesystem;

// map<blockName, textureID>
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

    void initialize(const fs::path& folderPath) {
        for (auto& entry : fs::directory_iterator(folderPath)) {
            if (!entry.is_regular_file()) continue;
            auto path = entry.path();
            if (path.extension() != ".png") continue;

            std::string name = path.stem().string();
            GLuint texID = loadTexture(path);
            if (texID != 0) {
                textures[name] = texID;
            } else {
                std::cerr << "Failed to load texture: " << path << std::endl;
            }
        }
    }

    void initializeTextures(GLuint shaderProgramID) {
        auto textureIDs = TextureController::getInstance().getAllTextureIDs();

        for (GLuint i = 0; i < textureIDs.size(); ++i) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, textureIDs[i]);
        }

        GLint location = glGetUniformLocation(shaderProgramID, "textures");
        if (location == -1) {
            Logger::getInstance().Log("Warning: uniform 'textures' not found in shader", LogLevel::Warning, LogOutput::Both);
            return;
        }

        std::vector<GLint> samplers(textureIDs.size());
        for (int i = 0; i < samplers.size(); ++i)
            samplers[i] = i; // textures[0] => GL_TEXTURE0, textures[1] => GL_TEXTURE1, ...

        glUseProgram(shaderProgramID);
        glUniform1iv(location, samplers.size(), samplers.data());
        glUseProgram(0);
    }

    // get texture id. returns 0 if no texture
    GLuint getTextureID(const std::string& blockName) const {
        auto it = textures.find(blockName);
        if (it != textures.end()) return it->second;
        return 0;
    }

    std::vector<GLuint> getAllTextureIDs() const {
        std::vector<GLuint> ids;
        ids.reserve(textures.size());
        for (auto& kv : textures) {
            ids.push_back(kv.second);
        }
        return ids;
    }

    GLuint getTextureIDByIndex(size_t index) const {
        if (index >= textures.size()) throw std::out_of_range("Invalid texture index");
        auto it = textures.begin();
        std::advance(it, index);
        return it->second;
    }

    size_t getTextureCount() const {
        return textures.size();
    }

private:
    TextureController() = default;
    ~TextureController() = default;

    std::unordered_map<std::string, GLuint> textures;

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
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);

        Logger::getInstance().Log("Loaded texture: " + path.string() + " ID: " + std::to_string(textureID));

        return textureID;
    }
};
