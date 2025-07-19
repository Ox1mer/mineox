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

    Chunk(const ChunkPos& pos)
    : blocks(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE), _mesh(*this), chunkPos(pos)
    {
        for (int y = 0; y < CHUNK_SIZE; ++y) {
            for (int z = 0; z < CHUNK_SIZE; ++z) {
                for (int x = 0; x < CHUNK_SIZE; ++x) {
                    BlockPos pos{ glm::ivec3(x, y, z) };
                    blocks[toIndex(pos)] = BlockFactory::getInstance().create(Blocks::Air);
                    blocks[toIndex(pos)]->onPlace();
                }
            }
        }
    }

    void markSavingStarted() {
        canBeDeleted.store(false, std::memory_order_release);
    }

    void markSavingFinished() {
        canBeDeleted.store(true, std::memory_order_release);
    }

    bool isDeletable() const {
        return canBeDeleted.load(std::memory_order_acquire);
    }

    std::unique_ptr<Block>& getBlockPtr(BlockPos pos) {
        return blocks[toIndex(pos)];
    }

    const std::unique_ptr<Block>& getBlockPtr(BlockPos pos) const {
        return blocks[toIndex(pos)];
    }

    Block& getBlock(BlockPos pos) {
        return *blocks[toIndex(pos)];
    }

    const Block& getBlock(BlockPos pos) const {
        return *blocks[toIndex(pos)];
    }

    void setBlock(BlockPos pos, const Blocks blockType) {
        blocks[toIndex(pos)] = BlockFactory::getInstance().create(blockType);
        blocks[toIndex(pos)]->onPlace();
        markChunkDirty();
    }

    void breakBlock(BlockPos pos) {
        blocks[toIndex(pos)]->onBreak();
        blocks[toIndex(pos)] = BlockFactory::getInstance().create(Blocks::Air);
        markChunkDirty();
    }

    const std::vector<std::unique_ptr<Block>>& getBlocks() const {
        return blocks;
    }

    void updateChunkBlocksOpaqueData() {
        for (int y = 0; y < CHUNK_SIZE; ++y) {
            for (int z = 0; z < CHUNK_SIZE; ++z) {
                for (int x = 0; x < CHUNK_SIZE; ++x) {
                    int index = x + CHUNK_SIZE * (y + CHUNK_SIZE * z);
                    Block& block = *blocks[index];
                    blocksOpaqueData.setOpaque(x, y, z, !block.isTransparent());
                }
            }
        }
    }

    ChunkBlocksOpaqueData* getBlocksOpaqueData() {
        return &blocksOpaqueData;
    }

    void render(Shader shader) {
        _mesh.render(shader);
    }

    void markChunkDirty() {
        _mesh.needUpdate = true;
        _mesh.isUploaded = false;
    }

    int toIndex(BlockPos pos) const {
        return pos.position.x + CHUNK_SIZE * (pos.position.y + CHUNK_SIZE * pos.position.z);
    }

    const ChunkPos getChunkPos() const {
        return chunkPos;
    }

    void setBlocks(const std::vector<std::pair<BlockPos, Blocks>>& changes) {
        for (const auto& [pos, blockType] : changes) {
            int idx = toIndex(pos);
            if (idx < 0 || idx >= (int)blocks.size()) continue;

            blocks[idx] = BlockFactory::getInstance().create(blockType);
            blocks[idx]->onPlace();
        }
        markChunkDirty();
    }


private:
    std::vector<std::unique_ptr<Block>> blocks;
    ChunkMesh _mesh;
    ChunkBlocksOpaqueData blocksOpaqueData;
    ChunkPos chunkPos;
};
