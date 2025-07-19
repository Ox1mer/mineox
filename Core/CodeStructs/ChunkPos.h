#pragma once

#include <glm/glm.hpp>
#include <string>

struct ChunkPos {
    glm::ivec3 position;

    ChunkPos(int x, int y, int z)
        : position(x, y, z) {}

    ChunkPos(glm::ivec3 pos)
        : position(pos) {}

    ChunkPos()
        : position(0, 0, 0) {}

    bool operator==(const ChunkPos& other) const {
        return position == other.position;
    }

    std::string toDebugString() const {
        return "ChunkPos [ X: " + std::to_string(position.x) + ", Y: " + std::to_string(position.y) + ", Z: " + std::to_string(position.z) + "]";
    }

    std::string toString() const {
        return "Chunk_" + std::to_string(position.x) + "_" + std::to_string(position.y) + "_" + std::to_string(position.z);
    }
};

namespace std {
    template<>
    struct hash<ChunkPos> {
        size_t operator()(const ChunkPos& cp) const noexcept {
            size_t hx = std::hash<int>{}(cp.position.x);
            size_t hy = std::hash<int>{}(cp.position.y);
            size_t hz = std::hash<int>{}(cp.position.z);
            return hx ^ (hy << 1) ^ (hz << 2);
        }
    };
}
