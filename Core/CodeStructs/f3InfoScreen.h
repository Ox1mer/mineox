#pragma once

#include <glm/glm.hpp>
#include <string>

#include "Camera.h"
#include "RayCastHit.h"
#include "World.h"
#include "FontsLoader.h"

struct f3InfoScreen
{
    int fps = 0;
    int viewDistance = 0;
    glm::ivec3 playerPos{};
    glm::ivec3 chunkPos{};
    glm::ivec3 blockPos{};
    std::string facedBlockInfo;

    void update(float deltaTime, const Camera& camera, World& world, const std::optional<RaycastHit>& raycastHit) {
        fps = static_cast<int>(1.0f / deltaTime);
        viewDistance = world.getViewDistance();
        playerPos = glm::ivec3(camera.Position);
        
        chunkPos = world.getChunkController().toChunkPos(playerPos).position;
        blockPos = playerPos;

        BlockPos hitBlockPos = raycastHit.has_value()
            ? BlockPos(raycastHit->blockPos)
            : BlockPos(glm::ivec3(0));

        auto block = world.getChunkController().getBlock(hitBlockPos);

        if (block) {
            facedBlockInfo = raycastHit.has_value()
                ? "Block: " + block.value().get().getStringRepresentation() +
                  ", Face Normal: " + toString(raycastHit->faceNormal)
                : "No block hit";
        } else {
            facedBlockInfo = "No block hit";
        }
    }

    static std::string toString(const glm::vec3& vec) {
        return std::to_string(vec.x) + ", " + std::to_string(vec.y) + ", " + std::to_string(vec.z);
    }

    static std::string toString(const glm::ivec3& vec) {
        return std::to_string(vec.x) + ", " + std::to_string(vec.y) + ", " + std::to_string(vec.z);
    }

    static std::string toString(int value) {
        return std::to_string(value);
    }

    static std::string toString(const glm::ivec3& vec, const std::string& prefix) {
        return prefix + " " + std::to_string(vec.x) + ", " + std::to_string(vec.y) + ", " + std::to_string(vec.z);
    }

    void render(FontsLoader& fontsLoader,
            Shader& fontsShader,
            float windowHeight,
            const glm::vec3& color = glm::vec3(0.0f, 0.0f, 0.0f)) const
    {
        float startX = 10.0f;
        float startY = windowHeight - 30.0f;
        float lineHeight = 30.0f;

        auto drawLine = [&](const std::string& text, int line) {
            fontsLoader.RenderText(
                fontsShader,
                text,
                startX,
                startY - lineHeight * line,
                1.0f,
                color,
                fontsLoader.fontProjection
            );
        };

        drawLine("FPS: " + toString(fps), 0);
        drawLine(toString(blockPos, "Block Pos:"), 1);
        drawLine(toString(chunkPos, "Chunk Pos:"), 2);
        drawLine(toString(playerPos, "Coords:"), 3);
        drawLine("View distance: " + toString(viewDistance), 4);
        drawLine(facedBlockInfo, 5);
    }
};