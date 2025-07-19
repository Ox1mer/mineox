#pragma once

#include "ChunkPos.h"

struct ChunkPosHash {
    size_t operator()(const ChunkPos& pos) const noexcept {
        size_t h1 = std::hash<int>{}(pos.position.x);
        size_t h2 = std::hash<int>{}(pos.position.y);
        size_t h3 = std::hash<int>{}(pos.position.z);
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};
