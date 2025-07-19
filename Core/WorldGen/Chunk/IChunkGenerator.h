#pragma once

#include "ChunkPos.h"

enum class generationType {
    Flat,
    Noise,
    Custom
};

class IChunkGenerator {
public:
    virtual ~IChunkGenerator() = default;

    virtual void generateChunk(const ChunkPos& pos) = 0;

    virtual generationType getGeneratorType() const = 0;

    virtual bool isReady() const = 0;
};

// For now dont used. Need to realize later.