#pragma once

#include <memory>
#include <vector>

#include "SunShadow.h"
#include "Chunk.h"
#include "Shader.h"

class ShadowController {
private:
    std::unique_ptr<SunShadow> sunShadow;

public:
    ShadowController() = default;

    ~ShadowController() {
        cleanup();
    }

    void init() {
        sunShadow = std::make_unique<SunShadow>(4096);
        sunShadow->init();
    }

    void update(const glm::vec3& lightDirection, const glm::vec3& center, int viewDistance) {
        if (sunShadow) {
            sunShadow->update(lightDirection, center, viewDistance);
        }
    }

    void renderShadows(const std::unordered_map<ChunkPos, std::unique_ptr<Chunk>>& loadedChunks, Shader& depthShader) {
        if (!sunShadow) return;

        std::vector<Chunk*> visibleChunks;
        visibleChunks.reserve(loadedChunks.size());

        for (const auto& [pos, chunkPtr] : loadedChunks) {
            if (chunkPtr) {
                visibleChunks.push_back(chunkPtr.get());
            }
        }

        sunShadow->render(visibleChunks, depthShader);
    }

    void cleanup() {
        if (sunShadow) {
            sunShadow->cleanup();
        }
    }

    unsigned int getSunShadowMap() const {
        return sunShadow ? sunShadow->getDepthMap() : 0;
    }

    const glm::mat4& getSunLightSpaceMatrix() const {
        static glm::mat4 identity = glm::mat4(1.0f);
        return sunShadow ? sunShadow->getLightSpaceMatrix() : identity;
    }
};