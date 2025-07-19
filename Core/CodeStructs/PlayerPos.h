#pragma once

#include <glm/glm.hpp>
#include <string>

struct PlayerPos
{
    glm::ivec3 position;

    bool operator==(const PlayerPos& other) const {
        return position == other.position;
    }

    std::string toString() const {
        return "PlayerPos [ X: " + std::to_string(position.x) + ", Y: " + std::to_string(position.y) + ", Z: " + std::to_string(position.z) + "]";
    }
};
