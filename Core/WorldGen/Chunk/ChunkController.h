#pragma once

#include <memory>
#include <optional>
#include <string>
#include <unordered_set>
#include <mutex>

#include "ChunkPos.h"
#include "ChunkPosHash.h"
#include "Chunk.h"
#include "Block.h"
#include "Blocks.h"
#include "BlockPos.h"
#include "ChunkMemoryContainer.h"
#include "ChunkDataAccess.h"
#include "ThreadPool.h"
#include "Logger.h"
#include "EventBus.h"
#include "ChunkSavedEvent.h"
#include "Shader.h"

class ChunkController {
public:
    explicit ChunkController(const std::string& worldName)
        : worldName(worldName),
          _chunkMemoryContainer(std::make_unique<ChunkMemoryContainer>()),
          _chunkDataAccess(std::make_unique<ChunkDataAccess>()) {}

    // === Chunk Access ===
    std::optional<std::reference_wrapper<Chunk>> getChunk(const ChunkPos& pos) const;
    bool hasChunk(const ChunkPos& pos) const;

    // === Block Operations ===
    void setBlock(const BlockPos& pos, Blocks id);
    std::optional<std::reference_wrapper<Block>> getBlock(const BlockPos& pos) const;
    void breakBlock(const BlockPos& pos);

    // === Update & Render ===
    void renderAllChunks(Shader& shader, const glm::vec3& sunDirection, const glm::vec3& sunColor);
    void renderChunk(const ChunkPos& pos, Shader& shader, const glm::vec3& sunDirection, const glm::vec3& sunColor);
    void markChunkDirty(const ChunkPos& pos);
    void updateChunk(const ChunkPos& pos, float deltaTime);
    void update(const glm::ivec3& playerPos, int viewDistance);

    // === Utility ===
    ChunkPos toChunkPos(const glm::ivec3& pos) const;

private:
    glm::ivec3 worldToChunk(const glm::ivec3& worldPos) const;

    std::string worldName;

    std::unique_ptr<ChunkMemoryContainer> _chunkMemoryContainer;
    std::unique_ptr<ChunkDataAccess> _chunkDataAccess;

    std::unordered_set<ChunkPos> _loadingSet;
    std::mutex _loadingMutex;

    std::optional<ChunkPos> _lastCenter;
};
