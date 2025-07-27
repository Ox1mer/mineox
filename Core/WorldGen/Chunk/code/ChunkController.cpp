#include "ChunkController.h"

std::optional<std::reference_wrapper<Chunk>> ChunkController::getChunk(const ChunkPos& pos) const {
    return _chunkMemoryContainer->getChunk(pos);
}

bool ChunkController::hasChunk(const ChunkPos& pos) const {
    return _chunkMemoryContainer->getChunk(pos).has_value();
}

int floorDiv(int a, int b) {
    return (a >= 0) ? (a / b) : ((a - b + 1) / b);
}

glm::ivec3 ChunkController::worldToChunk(const glm::ivec3& worldPos) const {
    return glm::ivec3(
        floorDiv(worldPos.x, Chunk::CHUNK_SIZE),
        floorDiv(worldPos.y, Chunk::CHUNK_SIZE),
        floorDiv(worldPos.z, Chunk::CHUNK_SIZE)
    );
}

void ChunkController::setBlock(const BlockPos& pos, Blocks id) {
    auto chunkOpt = getChunk(ChunkPos(worldToChunk(pos.position)));
    if (chunkOpt) {
        auto toLocal = [](int globalCoord) {
            int local = globalCoord % Chunk::CHUNK_SIZE;
            if (local < 0) local += Chunk::CHUNK_SIZE;
            return local;
        };

        glm::ivec3 localPos{
            toLocal(pos.position.x),
            toLocal(pos.position.y),
            toLocal(pos.position.z)
        };

        chunkOpt->get().setBlock(BlockPos(localPos), id);
    }
}

std::optional<std::reference_wrapper<Block>> ChunkController::getBlock(const BlockPos& pos) const {
    auto chunkOpt = getChunk(ChunkPos(worldToChunk(pos.position)));
    if (chunkOpt) {

        auto toLocal = [](int globalCoord) {
            int local = globalCoord % Chunk::CHUNK_SIZE;
            if (local < 0) local += Chunk::CHUNK_SIZE;
            return local;
        };

        glm::ivec3 localPos{
            toLocal(pos.position.x),
            toLocal(pos.position.y),
            toLocal(pos.position.z)
        };

        return chunkOpt->get().getBlock(BlockPos(localPos));
    }
    return std::nullopt;
}

void ChunkController::breakBlock(const BlockPos& pos) {
    auto chunkOpt = getChunk(ChunkPos(worldToChunk(pos.position)));
    if (chunkOpt) {
        auto toLocal = [](int globalCoord) {
            int local = globalCoord % Chunk::CHUNK_SIZE;
            if (local < 0) local += Chunk::CHUNK_SIZE;
            return local;
        };

        glm::ivec3 localPos{
            toLocal(pos.position.x),
            toLocal(pos.position.y),
            toLocal(pos.position.z)
        };

        chunkOpt->get().breakBlock(BlockPos(localPos));
    }
}

void ChunkController::renderAllChunks(Shader& shader, const glm::vec3& sunDirection, const glm::vec3& sunColor) {
    auto chunkPositions = _chunkMemoryContainer->getLoadedChunksPosition();
    for (const auto& pos : chunkPositions) {
        renderChunk(pos, shader, sunDirection, sunColor);
    }
}

void ChunkController::renderChunk(const ChunkPos& pos, Shader& shader, const glm::vec3& sunDirection, const glm::vec3& sunColor) {
    auto chunkOpt = getChunk(pos);
    if (chunkOpt) {
        chunkOpt->get().render(shader, sunDirection, sunColor);
    }
}

void ChunkController::markChunkDirty(const ChunkPos& pos) {
    if (getChunk(pos).has_value()) {
        getChunk(pos).value().get().markChunkDirty();
    }
}

void ChunkController::updateChunk(const ChunkPos& pos, float deltaTime) {
    markChunkDirty(pos);
}

int divFloor(int a, int b) {
    int q = a / b;
    int r = a % b;
    if ((r != 0) && ((r < 0) != (b < 0))) {
        q -= 1;
    }
    return q;
}

ChunkPos ChunkController::toChunkPos(const glm::ivec3& pos) const {
    ChunkPos chunkPos;
    chunkPos.position = glm::ivec3(
        divFloor(pos.x, Chunk::CHUNK_SIZE),
        divFloor(pos.y, Chunk::CHUNK_SIZE),
        divFloor(pos.z, Chunk::CHUNK_SIZE)
    );
    return chunkPos;
}

void ChunkController::update(const glm::ivec3& playerPos, int viewDistance) {
    auto center = toChunkPos(playerPos);
    auto prevPos = _lastCenter;
    if (_lastCenter.has_value()) {
        if (_lastCenter.value() == center) {
            return;
        }
    }
    _lastCenter = center;

    std::vector<ChunkPos> listOfChunkPositionsAroundCenter;

    for (int x = center.position.x - viewDistance; x <= center.position.x + viewDistance; ++x) {
        for (int y = center.position.y - viewDistance; y <= center.position.y + viewDistance; ++y) {
            for (int z = center.position.z - viewDistance; z <= center.position.z + viewDistance; ++z) {
                listOfChunkPositionsAroundCenter.push_back(ChunkPos{ glm::ivec3(x, y, z) });
            }
        }
    }

    _chunkMemoryContainer->loadVectorOfChunks(listOfChunkPositionsAroundCenter, worldName);
}