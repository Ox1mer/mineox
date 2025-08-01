#pragma once

#include <vector>
#include <atomic>
#include <memory>

#include "Blocks.h"
#include "BlockFactory.h"
#include "ChunkBlocksOpaqueData.h"
#include "BlockPos.h"
#include "Shader.h"
#include "ChunkMesh.h"

class Chunk {
public:
    static constexpr int CHUNK_SIZE = 32;

    std::atomic<bool> canBeDeleted{true};

    Chunk(const ChunkPos& pos);

    void markSavingStarted();
    void markSavingFinished();
    bool isDeletable() const;

    std::shared_ptr<Block>& getBlockPtr(BlockPos pos);
    const std::shared_ptr<Block>& getBlockPtr(BlockPos pos) const;

    Block& getBlock(BlockPos pos);
    const Block& getBlock(BlockPos pos) const;

    void setBlock(BlockPos pos, const Blocks blockType);
    void breakBlock(BlockPos pos);

    const std::vector<std::shared_ptr<Block>>& getBlocks() const;

    void updateChunkBlocksOpaqueData();
    ChunkBlocksOpaqueData* getBlocksOpaqueData();

    void render(Shader shader, const glm::vec3& sunDirection, const glm::vec3& sunColor);

    void markChunkDirty();

    int toIndex(BlockPos pos) const;

    const ChunkPos getChunkPos() const;

    void setBlocks(const std::vector<std::pair<BlockPos, Blocks>>& changes);

    void renderDepth(Shader& depthShader);

    void updateNearChunks(glm::ivec3 localPos);
    void updateNeighborMesh(glm::ivec3 offset);

private:
    std::vector<std::shared_ptr<Block>> blocks;
    ChunkMesh _mesh;
    ChunkBlocksOpaqueData blocksOpaqueData;
    ChunkPos chunkPos;
};
