#pragma once
#include <glm/glm.hpp>
#include <string>

struct f3InfoScreen
{
    int fps;
    glm::ivec3 playerPos;
    glm::ivec3 chunkPos;
    glm::ivec3 blockPos;
    std::string facedBlockInfo;

    std::string toString(glm::ivec3 pos) const {
        return "X: " + std::to_string(pos.x) + ", Y: " + std::to_string(pos.y) + ", Z: " + std::to_string(pos.z);
    }

    std::string toString(int value) const {
        return std::to_string(value);
    }
};
