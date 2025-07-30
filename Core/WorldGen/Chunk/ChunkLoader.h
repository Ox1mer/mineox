#pragma once

#include <string>

#include "ChunkPos.h"
#include "FileHandler.h"
#include "PathProvider.h"
#include "Chunk.h"
#include "memory"
#include "ChunkDataAccess.h"
#include "Blocks.h"
#include "ScopedTimer.h"

#include <random>

class ChunkLoader {
public:

    ChunkLoader() {
        _chunkDataAccess = ChunkDataAccess();
    }

    std::unique_ptr<Chunk> loadChunk(const ChunkPos& chunkPos, const std::string& worldName) {
        auto chunkOpt = _chunkDataAccess.loadChunkFromDisk(chunkPos, worldName);
        if (chunkOpt.has_value()) {
            return std::move(chunkOpt.value());
        } else {
            return nullptr;
        }
    }

    void saveChunk(const ChunkPos& chunkPos, const Chunk& chunk, const std::string& worldName) {
        _chunkDataAccess.saveChunkToDisk(chunkPos, chunk, worldName);
    }
    
    int getSurfaceHeight(int x, int z) {
        float height = 3.0f + 2.0f * sinf(x * 0.3f) * cosf(z * 0.3f);
        return static_cast<int>(height);
    }

    std::unique_ptr<Chunk> generateChunk(ChunkPos chunkPos) {
        auto chunk = std::make_unique<Chunk>(chunkPos);
        std::vector<std::pair<BlockPos, Blocks>> blocks;

        constexpr int size = Chunk::CHUNK_SIZE;

        if(chunk->getChunkPos().position.y == 0) {for (int x = 0; x < size; ++x) {
            for (int z = 0; z < size; ++z) {

                int surfaceHeight = getSurfaceHeight(x, z);
                
                for (int y = 0; y < size; ++y) {
                    Blocks block = Blocks::Air; // по умолчанию воздух

                    if (y == 0) {
                        block = Blocks::Stone; // самый низ - камень
                    } else if (y < surfaceHeight - 2) {
                        block = Blocks::Gneiss; // слой гнейса под землёй
                    } else if (y < surfaceHeight - 1) {
                        block = Blocks::Dirt; // земля под поверхностью
                    } else if (y == surfaceHeight - 1) {
                        // Верхний слой - с шансом песок или гравий, либо споровый мох
                        static std::mt19937 rng(42 + chunkPos.position.x * 73856093 + chunkPos.position.z * 19349663 + x * 83492791 + z * 1234567);
                        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
                        float r = dist(rng);

                        if (r < 0.1f) {
                            block = Blocks::Sand;
                        } else if (r < 0.15f) {
                            block = Blocks::Gravel;
                        } else if (r < 0.2f) {
                            block = Blocks::SporeMoss;
                        } else {
                            block = Blocks::Dirt;
                        }
                    }
                    // Немного воздуха внутри - маленькие пещеры
                    else if (y < surfaceHeight && (x + z + y) % 7 == 0) {
                        block = Blocks::Air;
                    } else if (y < surfaceHeight) {
                        block = Blocks::Dirt;
                    }

                    if (block != Blocks::Air) {
                        blocks.emplace_back(BlockPos(glm::ivec3(x, y, z)), block);
                    }
                }
            }
        }

        chunk->setBlocks(blocks);}

        return chunk;
    }

private:
    ChunkDataAccess _chunkDataAccess;
};