#include "ChunkMemoryContainer.h"

std::optional<std::reference_wrapper<Chunk>>
ChunkMemoryContainer::getChunk(const ChunkPos& pos) const {
    std::shared_lock lock(_mutex);
    auto it = _chunks.find(pos);
    if (it == _chunks.end()) {
        return std::nullopt;
    }
    return std::ref(*it->second);
}

void ChunkMemoryContainer::removeChunk(const ChunkPos& pos) {
    std::unique_lock lock(_mutex);
    auto it = _chunks.find(pos);
    if (it == _chunks.end()) return;
    _chunks.erase(it);
}

void ChunkMemoryContainer::loadChunk(const ChunkPos& pos, std::unique_ptr<Chunk> chunk) {
    std::unique_lock lock(_mutex);
    auto [it, inserted] = _chunks.emplace(pos, std::move(chunk));
    if (!inserted) {
        Logger::getInstance().Log(
            "Chunk already loaded at position: " + pos.toString(),
            LogLevel::Warning,
            LogOutput::Both,
            LogWriteMode::Append
        );
        return;
    }
}

void ChunkMemoryContainer::unloadChunk(const ChunkPos& pos) {
    {
        std::unique_lock lock(_mutex);
        auto it = _chunks.find(pos);
        if (it == _chunks.end()) {
            Logger::getInstance().Log(
                "Chunk not found at position: " + pos.toString(),
                LogLevel::Warning,
                LogOutput::Both,
                LogWriteMode::Append
            );
            return;
        }
        _chunks.erase(it);
    }
}

std::vector<ChunkPos> ChunkMemoryContainer::getLoadedChunksPosition() const {
    std::shared_lock lock(_mutex);
    std::vector<ChunkPos> positions;
    positions.reserve(_chunks.size());
    for (auto const& [pos, _] : _chunks) {
        positions.push_back(pos);
    }
    return positions;
}

void ChunkMemoryContainer::loadVectorOfChunks(const std::vector<ChunkPos>& chunksPos, const std::string& worldName) {
    std::vector<ChunkPos> toLoad;

    {
        std::shared_lock lock(_mutex);
        for (const auto& chunkPos : chunksPos) {
            if (!getChunk(chunkPos).has_value() && !_loadingSet.contains(chunkPos)) {
                toLoad.push_back(chunkPos);
            }
        }
    }

    {
        std::unique_lock lock(_mutex);
        for (const auto& chunkPos : toLoad) {
            _loadingSet.insert(chunkPos);
        }
    }

    const size_t batchSize = 5;

    for (size_t i = 0; i < toLoad.size(); i += batchSize) {
        std::vector<std::pair<ChunkPos, bool>> batch;
        size_t end = std::min(i + batchSize, toLoad.size());

        for (size_t j = i; j < end; ++j) {
            const auto& chunkPos = toLoad[j];
            bool exists = FileHandler::getInstance().fileExists(PathProvider::getInstance().getChunkFilePath(worldName, chunkPos));
            batch.emplace_back(chunkPos, exists);
        }

        ThreadPool::getInstance().enqueueChunkTask([this, batch = std::move(batch), worldName]() mutable {
            for (auto& [chunkPos, exists] : batch) {
                std::unique_ptr<Chunk> chunk;

                if (exists) {
                    chunk = _chunkLoader.loadChunk(chunkPos, worldName);
                } else {
                    chunk = _chunkLoader.generateChunk(chunkPos);
                }

                if (chunk) {
                    std::unique_lock lock(_mutex);
                    auto [it, inserted] = _chunks.emplace(chunkPos, std::move(chunk));
                    if (!inserted) {
                        Logger::getInstance().Log("Chunk already loaded", LogLevel::Warning);
                    }
                    _loadingSet.erase(chunkPos);
                }
            }
        });
    }
}
