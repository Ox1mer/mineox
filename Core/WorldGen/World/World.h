#pragma once

#include <string>

#include "ChunkController.h"
#include "PlayerPos.h"
#include "Shader.h"
#include "RayCastHit.h"
#include "ShadowController.h"
#include "TimeOfDayController.h"

#include "BlockCache.h"

class World {
private:
    int seed;
    std::string worldName;
    ChunkController _chunkController;
    ShadowController _shadowController;
    TimeOfDayController _timeOfDayController;
    BlockCache& _blockCache = BlockCache::getInstance();
    int viewDistance = 5;

public:
    World(int seed, std::string worldName)
        : seed(seed), worldName(worldName), _chunkController(worldName) {

        }

    ~World() {}

    void update(PlayerPos plPos) {
        _chunkController.update(plPos.position, viewDistance);
    }

    void render(Shader& shader, const glm::vec3& sunDirection, const glm::vec3& sunColor) {
        _chunkController.renderAllChunks(shader, sunDirection, sunColor);
    }

    void save() {}

    ChunkController& getChunkController() { return _chunkController; }
    int getSeed() const { return seed; }
    const std::string& getWorldName() const { return worldName; }
    TimeOfDayController& getTimeOfDayController() { return _timeOfDayController; }

    int getViewDistance() const { return viewDistance; }
    void setViewDistance(int distance) {
        if (distance > 0) {
            viewDistance = distance;
        }
    }

    ShadowController& getShadowController() { return _shadowController; }

    std::optional<RaycastHit> raycast(const glm::vec3& startPos, const glm::vec3& dir, float maxDistance) {
        glm::vec3 pos = startPos;

        glm::ivec3 blockPos = glm::floor(pos);

        glm::ivec3 step = {
            (dir.x > 0) ? 1 : (dir.x < 0) ? -1 : 0,
            (dir.y > 0) ? 1 : (dir.y < 0) ? -1 : 0,
            (dir.z > 0) ? 1 : (dir.z < 0) ? -1 : 0
        };

        glm::vec3 nextBoundary = glm::vec3(blockPos) + glm::vec3((step.x > 0) ? 1.0f : 0.0f, 
                                                                 (step.y > 0) ? 1.0f : 0.0f, 
                                                                 (step.z > 0) ? 1.0f : 0.0f);

        glm::vec3 tMax = (nextBoundary - pos) / dir;

        if (dir.x == 0) tMax.x = std::numeric_limits<float>::infinity();
        if (dir.y == 0) tMax.y = std::numeric_limits<float>::infinity();
        if (dir.z == 0) tMax.z = std::numeric_limits<float>::infinity();

        glm::vec3 tDelta = glm::vec3(
            (step.x != 0) ? (1.0f / std::abs(dir.x)) : std::numeric_limits<float>::infinity(),
            (step.y != 0) ? (1.0f / std::abs(dir.y)) : std::numeric_limits<float>::infinity(),
            (step.z != 0) ? (1.0f / std::abs(dir.z)) : std::numeric_limits<float>::infinity()
        );

        float traveled = 0.0f;

        glm::ivec3 faceNormal = glm::ivec3(0);

        while (traveled <= maxDistance) {
            auto block = _chunkController.getBlock(BlockPos(blockPos));
            if (block.has_value()) {
                if (_blockCache.getBlockInfo(block.value().get().getBlockId()).isSolid) {
                    return RaycastHit{ blockPos, faceNormal };
                }
            }
            if (tMax.x < tMax.y) {
                if (tMax.x < tMax.z) {
                    blockPos.x += step.x;
                    traveled = tMax.x;
                    tMax.x += tDelta.x;
                    faceNormal = glm::ivec3(-step.x, 0, 0);
                } else {
                    blockPos.z += step.z;
                    traveled = tMax.z;
                    tMax.z += tDelta.z;
                    faceNormal = glm::ivec3(0, 0, -step.z);
                }
            } else {
                if (tMax.y < tMax.z) {
                    blockPos.y += step.y;
                    traveled = tMax.y;
                    tMax.y += tDelta.y;
                    faceNormal = glm::ivec3(0, -step.y, 0);
                } else {
                    blockPos.z += step.z;
                    traveled = tMax.z;
                    tMax.z += tDelta.z;
                    faceNormal = glm::ivec3(0, 0, -step.z);
                }
            }
        }
        return std::nullopt;
    }

    void initWorld(const glm::vec3& playerPos) {
        _chunkController.initWorld(playerPos, viewDistance);
    }
};
