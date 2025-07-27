#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <functional>
#include <shared_mutex>
#include <optional>
#include <mutex>
#include <string>

#include "Chunk.h"
#include "ChunkPos.h"
#include "ChunkLoader.h"
#include "Logger.h"
#include "FileHandler.h"
#include "PathProvider.h"
#include "ThreadPool.h"

class ChunkMemoryContainer {
public:
    ChunkMemoryContainer() = default;
    ~ChunkMemoryContainer() = default;

    // === Chunk Access ===
    std::optional<std::reference_wrapper<Chunk>> getChunk(const ChunkPos& pos) const;
    std::vector<ChunkPos> getLoadedChunksPosition() const;

    // === Chunk Management ===
    void loadChunk(const ChunkPos& pos, std::unique_ptr<Chunk> chunk);
    void loadVectorOfChunks(const std::vector<ChunkPos>& chunksPos, const std::string& worldName);
    void removeUnlistedChunks(const std::vector<ChunkPos>& chunksPos, const std::string& worldName);

    void unloadChunk(const ChunkPos& pos);
    void removeChunk(const ChunkPos& pos);

    // === Callbacks ===
    void setSaveCallback(std::function<void(std::unique_ptr<Chunk>)> callback);

    // === Debug ===
    void logChunks() const;

    std::unordered_map<ChunkPos, std::unique_ptr<Chunk>>& getLoadedChunks() {
        return _chunks;
    }

private:
    mutable std::shared_mutex _mutex;
    std::mutex _loadingMutex;

    std::unordered_map<ChunkPos, std::unique_ptr<Chunk>> _chunks;
    std::unordered_set<ChunkPos> _loadingSet;

    ChunkLoader _chunkLoader;
    std::function<void(std::unique_ptr<Chunk>)> _saveCallback;
};
