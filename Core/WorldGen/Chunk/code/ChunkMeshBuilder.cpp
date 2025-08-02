#include "ChunkMeshBuilder.h"
#include "Chunk.h"
#include "Logger.h"
#include "PathProvider.h"
#include "ScopedTimer.h"
#include "ServiceLocator.h"
#include "TextureManager.h"

ChunkMeshBuilder::ChunkMeshBuilder(Chunk& chunk) : chunk(chunk) {}

void ChunkMeshBuilder::buildMesh() {
    clear();
    chunk.updateChunkBlocksOpaqueData();
    ChunkBlocksOpaqueData* opaque = chunk.getBlocksOpaqueData();

    const int s = Chunk::CHUNK_SIZE;

    for (int x = 0; x < s; ++x) {
        for (int y = 0; y < s; ++y) {
            for (int z = 0; z < s; ++z) {
                glm::ivec3 localPos(x, y, z);
                if (!getOpaqueSafe(opaque, localPos)) continue;

                for (int faceIndex = 0; faceIndex < 6; ++faceIndex) {
                    const Face& face = faces[faceIndex];
                    glm::ivec3 neighborPos = localPos + face.neighborOffset;

                    if (!getOpaqueSafe(opaque, neighborPos)) {
                        glm::ivec3 worldPos = chunk.getChunkPos().position * s + localPos;

                        if (face.normal.x < 0 || face.normal.y < 0 || face.normal.z < 0) {
                        } else {
                            // For the positive normal add 1
                            worldPos += glm::ivec3(face.normal);
                        }

                        int index = chunk.toIndex(BlockPos(localPos));
                        if (index < 0 || index >= (int)chunk.getBlocks().size()) continue;

                        auto& block = *chunk.getBlocks()[index];
                        std::string blockName = block.getStringRepresentation();

                        glm::ivec3 u, v;
                        if (face.normal.x != 0) {
                            u = {0, 1, 0};
                            v = {0, 0, 1};
                        } else if (face.normal.y != 0) {
                            u = {1, 0, 0};
                            v = {0, 0, 1};
                        } else {
                            u = {1, 0, 0};
                            v = {0, 1, 0};
                        }

                        std::string sideName = block.getBlockSidesTextureNames()[faceIndex];
                        const auto& atlasRegions = TextureManager::getInstance().getAtlasRegions();
                        auto it = atlasRegions.find(sideName);
                        if (it != atlasRegions.end()) {
                            const AtlasRegion& region = it->second;
                            addQuad(worldPos, u, v, face, 1, 1, region);
                        } else {
                            // If a texture is not found, you can substitute default UV coordinates or log an error message
                            Logger::getInstance().Log("Texture region not found for: " + sideName, LogLevel::Warning);
                            // For example, to avoid a crash, you can use a default region like {0, 0, 1, 1}
                            AtlasRegion defaultRegion{0.0f, 0.0f, 1.0f, 1.0f};
                            addQuad(worldPos, u, v, face, 1, 1, defaultRegion);
                        }
                    }
                }
            }
        }
    }
}

void ChunkMeshBuilder::addQuad(glm::ivec3 pos,
                               glm::ivec3 u,
                               glm::ivec3 v,
                               const Face& face,
                               int w,
                               int h,
                               const AtlasRegion region) {

    unsigned int startIndex = vertices.size();
    glm::vec3 normal = face.normal;

    glm::vec3 v0 = glm::vec3(pos);
    glm::vec3 v1 = v0 + glm::vec3(u) * float(w);
    glm::vec3 v2 = v1 + glm::vec3(v) * float(h);
    glm::vec3 v3 = v0 + glm::vec3(v) * float(h);

    glm::vec2 uv0 = {region.u0, region.v0};
    glm::vec2 uv1 = {region.u1, region.v0};
    glm::vec2 uv2 = {region.u1, region.v1};
    glm::vec2 uv3 = {region.u0, region.v1};

    vertices.push_back(Vertex{v0, normal, uv0});
    vertices.push_back(Vertex{v1, normal, uv1});
    vertices.push_back(Vertex{v2, normal, uv2});
    vertices.push_back(Vertex{v3, normal, uv3});

    glm::vec3 edge1 = v1 - v0;
    glm::vec3 edge2 = v3 - v0;
    glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));

    if (glm::dot(faceNormal, normal) < 0) {
        indices.push_back(startIndex + 0);
        indices.push_back(startIndex + 3);
        indices.push_back(startIndex + 2);

        indices.push_back(startIndex + 2);
        indices.push_back(startIndex + 1);
        indices.push_back(startIndex + 0);
    } else {
        indices.push_back(startIndex + 0);
        indices.push_back(startIndex + 1);
        indices.push_back(startIndex + 2);

        indices.push_back(startIndex + 2);
        indices.push_back(startIndex + 3);
        indices.push_back(startIndex + 0);
    }
}

bool ChunkMeshBuilder::getOpaqueSafe(ChunkBlocksOpaqueData* opaque, glm::ivec3 localPos) {
    int s = Chunk::CHUNK_SIZE;

    // В пределах текущего чанка
    if (localPos.x >= 0 && localPos.x < s &&
        localPos.y >= 0 && localPos.y < s &&
        localPos.z >= 0 && localPos.z < s) 
    {
        return opaque->isOpaque(localPos.x, localPos.y, localPos.z);
    }

    // Вышли за границы - ищем соседний чанк
    glm::ivec3 neighborChunkOffset(0);
    glm::ivec3 neighborLocalPos = localPos;

    if (localPos.x < 0) {
        neighborChunkOffset.x = -1;
        neighborLocalPos.x += s;
    } else if (localPos.x >= s) {
        neighborChunkOffset.x = 1;
        neighborLocalPos.x -= s;
    }

    if (localPos.y < 0) {
        neighborChunkOffset.y = -1;
        neighborLocalPos.y += s;
    } else if (localPos.y >= s) {
        neighborChunkOffset.y = 1;
        neighborLocalPos.y -= s;
    }

    if (localPos.z < 0) {
        neighborChunkOffset.z = -1;
        neighborLocalPos.z += s;
    } else if (localPos.z >= s) {
        neighborChunkOffset.z = 1;
        neighborLocalPos.z -= s;
    }

    // Получаем соседний чанк
    ChunkPos neighborChunkPos = chunk.getChunkPos();
    neighborChunkPos.position += neighborChunkOffset;

    auto neighborChunk = ServiceLocator::GetWorld()->getChunkController().getChunk(neighborChunkPos);
    if (!neighborChunk.has_value()) {
        // Соседнего чанка ещё нет - считаем пустым
        return false;
    }

    return !neighborChunk->get().getBlock(BlockPos(neighborLocalPos)).isTransparent();
}

void ChunkMeshBuilder::clear() {
    vertices.clear();
    indices.clear();
}

void ChunkMeshBuilder::update() {
    buildMesh();
}

size_t ChunkMeshBuilder::getVertexCount() const {
    return vertices.size();
}

size_t ChunkMeshBuilder::getIndexCount() const {
    return indices.size();
}