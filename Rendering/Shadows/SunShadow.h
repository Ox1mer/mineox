#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

#include "Chunk.h"
#include "Shader.h"

class SunShadow {
public:
    SunShadow(int resolution = 8192)
        : resolution(resolution) {}

    ~SunShadow() {
        cleanup();
    }

    void init() {
        initFramebuffer();
    }

    void update(const glm::vec3& lightDirection, const glm::vec3& center, int viewDistance) {
        computeLightSpaceMatrix(lightDirection, center, viewDistance);
    }

    void render(const std::vector<Chunk*>& visibleChunks, Shader& depthShader) {
        glViewport(0, 0, resolution, resolution);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        depthShader.use();
        depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        for (const auto& chunk : visibleChunks) {
            chunk->renderDepth(depthShader);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void cleanup() {
        if (depthMap) {
            glDeleteTextures(1, &depthMap);
            depthMap = 0;
        }
        if (depthMapFBO) {
            glDeleteFramebuffers(1, &depthMapFBO);
            depthMapFBO = 0;
        }
    }

    unsigned int getDepthMap() const { return depthMap; }
    const glm::mat4& getLightSpaceMatrix() const { return lightSpaceMatrix; }

    unsigned int getFBO() const { return depthMapFBO; }

private:
    unsigned int depthMapFBO = 0;
    unsigned int depthMap = 0;
    int resolution = 4096;

    glm::mat4 lightSpaceMatrix = glm::mat4(1.0f);

    void initFramebuffer() {
        glGenFramebuffers(1, &depthMapFBO);

        glGenTextures(1, &depthMap);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, resolution, resolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Error: Shadow map framebuffer is not complete!" << std::endl;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void computeLightSpaceMatrix(const glm::vec3& lightDir, const glm::vec3& center, int viewDistance) {
        float shadowDistance = 64.0f * viewDistance;

        float snapValue = 64.0f;
        glm::vec3 snappedCenter = glm::floor(center / snapValue) * snapValue;

        glm::vec3 lightPos = snappedCenter - lightDir * shadowDistance;

        glm::mat4 lightView = glm::lookAt(lightPos, snappedCenter, glm::vec3(0, 1, 0));
        glm::mat4 lightProj = glm::ortho(
            -shadowDistance, shadowDistance,
            -shadowDistance, shadowDistance,
            0.1f, shadowDistance * 2.0f
        );

        lightSpaceMatrix = lightProj * lightView;
    }

};
