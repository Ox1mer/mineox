#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"

class ChatController {
private:
    std::vector<std::string> messages;
    size_t maxMessages = 20;
    bool isVisible = false;

    Shader shader;

    GLuint quadVAO = 0;
    GLuint quadVBO = 0;

    float offsetX = 0.0f;
    float offsetY = 0.0f;

    std::string inputBuffer;

public:
    void setOffset(float x, float y) {
        offsetX = x;
        offsetY = y;
    }

    ChatController(Shader& shader, float offsetX = 0.0f, float offsetY = 0.0f)
        : shader(shader)
    {
        setOffset(offsetX, offsetY);
        init();
    }

    void init() {
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);

        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, nullptr, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

        glBindVertexArray(0);
    }

    bool getVisibility() {
        return isVisible;
    }

    void setvisible(bool visible) {
        isVisible = visible;
    }

    void render(float screenWidth, float screenHeight) {
        if (!isVisible) return;

        float width = screenWidth * 0.5f;
        float height = screenHeight * 0.5f;

        float x = offsetX;
        float y = offsetY;

        float vertices[] = {
            x, y,
            x + width, y,
            x + width, y + height,

            x, y,
            x + width, y + height,
            x, y + height
        };

        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glm::mat4 projection = glm::ortho(0.0f, screenWidth, 0.0f, screenHeight);

        shader.use();
        shader.setMat4("projection", projection);
        shader.setVec4("overlayColor", glm::vec4(1.0f, 1.0f, 1.0f, 0.5f));

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        glEnable(GL_DEPTH_TEST);
    }
};