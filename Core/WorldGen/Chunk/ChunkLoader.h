#pragma once

#include <string>

#include "ChunkPos.h"
#include "FileHandler.h"
#include "PathProvider.h"
#include "Chunk.h"
#include "memory"
#include "ChunkDataAccess.h"
#include "Blocks.h"

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
    
    std::unique_ptr<Chunk> generateChunk(ChunkPos chunkPos) {
        auto chunk = std::make_unique<Chunk>(chunkPos);
        std::vector<std::pair<BlockPos, Blocks>> dirtLayer;

        if (chunkPos.position.y == 0) {
            const int y = 1;

            for (int x = 0; x < Chunk::CHUNK_SIZE; ++x) {
                for (int z = 0; z < Chunk::CHUNK_SIZE; ++z) {
                    dirtLayer.emplace_back(BlockPos(glm::ivec3(x, y, z)), Blocks::Dirt);
                }
            }
        }

        chunk->setBlocks(dirtLayer);

        return chunk;
    }

private:
    ChunkDataAccess _chunkDataAccess;
};