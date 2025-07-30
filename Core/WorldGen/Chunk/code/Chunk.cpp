#include "Chunk.h"
#include "ServiceLocator.h"

#include <GL/glext.h>

Chunk::Chunk(const ChunkPos& pos)
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

void Chunk::markSavingStarted() {
    canBeDeleted.store(false, std::memory_order_release);
}

void Chunk::markSavingFinished() {
    canBeDeleted.store(true, std::memory_order_release);
}

bool Chunk::isDeletable() const {
    return canBeDeleted.load(std::memory_order_acquire);
}

std::unique_ptr<Block>& Chunk::getBlockPtr(BlockPos pos) {
    return blocks[toIndex(pos)];
}

const std::unique_ptr<Block>& Chunk::getBlockPtr(BlockPos pos) const {
    return blocks[toIndex(pos)];
}

Block& Chunk::getBlock(BlockPos pos) {
    return *blocks[toIndex(pos)];
}

const Block& Chunk::getBlock(BlockPos pos) const {
    return *blocks[toIndex(pos)];
}

void Chunk::setBlock(BlockPos pos, const Blocks blockType) {
    blocks[toIndex(pos)] = BlockFactory::getInstance().create(blockType);
    blocks[toIndex(pos)]->onPlace();
    markChunkDirty();
    updateNearChunks(pos.position);
}

void Chunk::breakBlock(BlockPos pos) {
    blocks[toIndex(pos)]->onBreak();
    blocks[toIndex(pos)] = BlockFactory::getInstance().create(Blocks::Air);
    markChunkDirty();
    updateNearChunks(pos.position);
}

const std::vector<std::unique_ptr<Block>>& Chunk::getBlocks() const {
    return blocks;
}

void Chunk::updateChunkBlocksOpaqueData() {
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

ChunkBlocksOpaqueData* Chunk::getBlocksOpaqueData() {
    return &blocksOpaqueData;
}

void Chunk::render(Shader shader, const glm::vec3& sunDirection, const glm::vec3& sunColor) {
    _mesh.render(shader, sunDirection, sunColor);
}

void Chunk::markChunkDirty() {
    _mesh.needUpdate = true;
    _mesh.isUploaded = false;
}

int Chunk::toIndex(BlockPos pos) const {
    return pos.position.x + CHUNK_SIZE * (pos.position.y + CHUNK_SIZE * pos.position.z);
}

const ChunkPos Chunk::getChunkPos() const {
    return chunkPos;
}

void Chunk::setBlocks(const std::vector<std::pair<BlockPos, Blocks>>& changes) {
    for (const auto& [pos, blockType] : changes) {
        int idx = toIndex(pos);
        if (idx < 0 || idx >= (int)blocks.size()) continue;

        blocks[idx] = BlockFactory::getInstance().create(blockType);
        blocks[idx]->onPlace();
        updateNearChunks(pos.position);
    }
    markChunkDirty();
}

void Chunk::renderDepth(Shader& depthShader) {
    if (_mesh.indexCount == 0 || _mesh.VAO == 0) return;

    depthShader.use();

    glm::mat4 modelMatrix = glm::mat4(1.0f);

    depthShader.setMat4("model", modelMatrix);

    glBindVertexArray(_mesh.VAO);
    glDrawElements(GL_TRIANGLES, _mesh.indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Chunk::updateNearChunks(glm::ivec3 localPos) {
    const int s = Chunk::CHUNK_SIZE;

    const bool atMinX = localPos.x == 0;
    const bool atMaxX = localPos.x == s - 1;
    const bool atMinY = localPos.y == 0;
    const bool atMaxY = localPos.y == s - 1;
    const bool atMinZ = localPos.z == 0;
    const bool atMaxZ = localPos.z == s - 1;

    if (!(atMinX || atMaxX || atMinY || atMaxY || atMinZ || atMaxZ)) {
        return;
    }

    if (atMinX) updateNeighborMesh({-1, 0, 0});
    if (atMaxX) updateNeighborMesh({ 1, 0, 0});
    if (atMinY) updateNeighborMesh({ 0,-1, 0});
    if (atMaxY) updateNeighborMesh({ 0, 1, 0});
    if (atMinZ) updateNeighborMesh({ 0, 0,-1});
    if (atMaxZ) updateNeighborMesh({ 0, 0, 1});
}

void Chunk::updateNeighborMesh(glm::ivec3 offset) {
    auto world = ServiceLocator::GetWorld();
    ChunkPos neighborPos = chunkPos;
    neighborPos.position += offset;

    auto neighbor = world->getChunkController().getChunk(neighborPos);
    if (neighbor.has_value()) {
        neighbor.value().get()._mesh.needUpdate = true;
        neighbor.value().get()._mesh.isUploaded = false;
        neighbor.value().get().updateChunkBlocksOpaqueData();
    }
}