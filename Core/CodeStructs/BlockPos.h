#pragma once

#include <glm/glm.hpp>
#include <string>

struct BlockPos
{
    glm::ivec3 position;

    bool operator==(const BlockPos& other) const {
        return position == other.position;
    }

    std::string toString() const {
        return "BlockPos [ X: " + std::to_string(position.x) + ", Y: " + std::to_string(position.y) + ", Z: " + std::to_string(position.z) + "]";
    }
};
