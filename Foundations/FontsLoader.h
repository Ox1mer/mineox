#pragma once

#include <stdexcept>
#include <map>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include "Logger.h"
#include "ft2build.h"
#include "FontChuructer.h"

class FontsLoader {
private:
    FT_Library ft;
    FT_Face face;
    std::map<char, Character> _characters;
    unsigned int VAO, VBO;
    float screenWidth, screenHeight;
public:
    glm::mat4 fontProjection;

    FontsLoader(float screenWidth, float screenHeight) : screenWidth(screenWidth), screenHeight(screenHeight) {
        // Initialize FreeType library
        if (FT_Init_FreeType(&ft)) {
            throw std::runtime_error("Could not initialize FreeType library");
        }
        fontProjection = glm::ortho(0.0f, screenWidth, 0.0f,  screenHeight);
        initVaoVbo();
    }

    ~FontsLoader() {
        FT_Done_Face(face);
        FT_Done_FreeType(ft);
        for (auto& [c, ch] : _characters) {
            glDeleteTextures(1, &ch.textureID);
        }
    }

    void loadFont(const std::string& fontPath) {
        if (!std::filesystem::exists(fontPath)) {
            throw std::runtime_error("Font file does not exist: " + fontPath);
        }
        if (FT_New_Face(ft, fontPath.c_str(), 0, &face)) {
            throw std::runtime_error("Could not load font: " + fontPath);
        }
    }

    void initVaoVbo() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void RenderText(Shader &s, std::string text, float x, float y, float scale, glm::vec3 color, glm::mat4& fontProjection)
    {
        s.use();
        s.setVec3("textColor", color);
        s.setInt("text", 0); 
        s.setMat4("projection", fontProjection);
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(VAO);

        std::string::const_iterator c;
        for (c = text.begin(); c != text.end(); c++)
        {
            Character ch = _characters[*c];

            float xpos = x + ch.Bearing.x * scale;
            float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

            float w = ch.Size.x * scale;
            float h = ch.Size.y * scale;
            float vertices[6][4] = {
                { xpos,     ypos + h,   0.0f, 0.0f },            
                { xpos,     ypos,       0.0f, 1.0f },
                { xpos + w, ypos,       1.0f, 1.0f },

                { xpos,     ypos + h,   0.0f, 0.0f },
                { xpos + w, ypos,       1.0f, 1.0f },
                { xpos + w, ypos + h,   1.0f, 0.0f }           
            };
            glBindTexture(GL_TEXTURE_2D, ch.textureID);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            x += (ch.Advance >> 6) * scale;
        }
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void loadCharacters(unsigned int fontSize) {
        FT_Set_Pixel_Sizes(face, 0, fontSize);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        
        for (unsigned char c = 0; c < 128; c++)
        {
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                Logger::getInstance().Log("Failed to load glyph: " + std::to_string(c), LogLevel::Warning, LogOutput::Both);
                continue;
            }
            unsigned int texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
            );
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            Character character = {
                texture, 
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<unsigned int>(face->glyph->advance.x)
            };
            _characters.insert(std::pair<char, Character>(c, character));
        }
        Logger::getInstance().Log("Font characters loaded successfully", LogLevel::Info, LogOutput::Both);
    }
};