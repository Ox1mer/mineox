#pragma once

#include <glad/glad.h>

#include "ChunkMeshBuilder.h"
#include "Chunk.h"
#include "Logger.h"
#include "GlResourceDeleter.h"
#include "TextureController.h"
#include "Shader.h"
#include "ScopedTimer.h"

class Chunk;

struct ChunkMesh
{
    ChunkMeshBuilder meshBuilder;
    GLuint VAO = 0;
    GLuint VBO = 0;
    GLuint EBO = 0;

    GLsizei vertexCount = 0;
    GLsizei indexCount = 0;

    bool isUploaded = true;
    bool needUpdate = false;
    bool texturesBound = false;
    Chunk& chunk;

    ChunkMesh(Chunk& chunk)
        : meshBuilder(chunk), chunk(chunk) {}

    ~ChunkMesh()
    {
        if (VAO) GLResourceDeleter::getInstance().queueDeleteVAO(VAO);
        if (VBO) GLResourceDeleter::getInstance().queueDeleteVBO(VBO);
        if (EBO) GLResourceDeleter::getInstance().queueDeleteEBO(EBO);
    }

    void uploadToGPU() {
        if (isUploaded) return;

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, meshBuilder.vertices.size() * sizeof(Vertex), meshBuilder.vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshBuilder.indices.size() * sizeof(unsigned int), meshBuilder.indices.data(), GL_STATIC_DRAW);

        constexpr GLuint posAttrib = 0;
        constexpr GLuint normalAttrib = 1;
        constexpr GLuint texCoordAttrib = 2;
        constexpr GLuint textureIDAttrib = 3;

        glEnableVertexAttribArray(posAttrib);
        glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

        glEnableVertexAttribArray(normalAttrib);
        glVertexAttribPointer(normalAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

        glEnableVertexAttribArray(texCoordAttrib);
        glVertexAttribPointer(texCoordAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));

        glEnableVertexAttribArray(textureIDAttrib);
        glVertexAttribIPointer(textureIDAttrib, 1, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, textureID));

        glBindVertexArray(0);

        vertexCount = static_cast<GLsizei>(meshBuilder.vertices.size());
        indexCount = static_cast<GLsizei>(meshBuilder.indices.size());

        isUploaded = true;
        needUpdate = false;
    }

    void update() {
        if (needUpdate) {
            meshBuilder.update();

            if (!isUploaded) {
                uploadToGPU();
            } else {
                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                glBufferData(GL_ARRAY_BUFFER, meshBuilder.vertices.size() * sizeof(Vertex), meshBuilder.vertices.data(), GL_STATIC_DRAW);

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshBuilder.indices.size() * sizeof(unsigned int), meshBuilder.indices.data(), GL_STATIC_DRAW);

                vertexCount = static_cast<GLsizei>(meshBuilder.vertices.size());
                indexCount = static_cast<GLsizei>(meshBuilder.indices.size());

                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

                needUpdate = false;
            }
        }
    }

    void render(Shader shader, const glm::vec3& sunDirection, const glm::vec3& sunColor) {
        if (needUpdate) update();
            shader.use();
            shader.setVec3("lightDir", sunDirection);
            shader.setVec3("lightColor", sunColor);

            if (!texturesBound) {
                auto& texCtrl = TextureController::getInstance();
                for (int i = 0; i < texCtrl.getTextureCount(); ++i) {
                    glActiveTexture(GL_TEXTURE0 + i);
                    glBindTexture(GL_TEXTURE_2D, texCtrl.getTextureIDByIndex(i));
                }
                int units[32];
                for (int i = 0; i < texCtrl.getTextureCount(); ++i) units[i] = i;
                glUniform1iv(glGetUniformLocation(shader.ID, "textures"), texCtrl.getTextureCount(), units);
                texturesBound = true;
            }

            glBindVertexArray(VAO);
            if (indexCount > 0)
                glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
            else
                glDrawArrays(GL_TRIANGLES, 0, vertexCount);
            glBindVertexArray(0);
        }

};