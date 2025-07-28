#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"
#include "Camera.h"

struct WireFrameCube
{
    unsigned int VAO{}, VBO{}, EBO{};

    static constexpr float vertices[] = {
        -0.5f, -0.5f, -0.5f, // 0
         0.5f, -0.5f, -0.5f, // 1
         0.5f,  0.5f, -0.5f, // 2
        -0.5f,  0.5f, -0.5f, // 3
        -0.5f, -0.5f,  0.5f, // 4
         0.5f, -0.5f,  0.5f, // 5
         0.5f,  0.5f,  0.5f, // 6
        -0.5f,  0.5f,  0.5f  // 7
    };

    static constexpr unsigned int indices[] = {
        0, 1, 1, 2, 2, 3, 3, 0, // Back face edges
        4, 5, 5, 6, 6, 7, 7, 4, // Front face edges
        0, 4, 1, 5, 2, 6, 3, 7  // Side edges
    };

    void init() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }

    void render(const glm::vec3& position,
                const Camera& camera,
                const glm::mat4& projection,
                Shader& shader,
                const glm::vec3& color = glm::vec3(0.0f, 1.0f, 0.0f)) const
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position + glm::vec3(0.5f));
        model = glm::scale(model, glm::vec3(1.01f));

        shader.use();
        shader.setMat4("model", model);
        shader.setMat4("view", camera.GetViewMatrix());
        shader.setMat4("projection", projection);
        shader.setVec3("color", color);

        float prevWidth;
        glGetFloatv(GL_LINE_WIDTH, &prevWidth);
        glLineWidth(2.0f);

        glBindVertexArray(VAO);
        glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glLineWidth(prevWidth);
    }
};