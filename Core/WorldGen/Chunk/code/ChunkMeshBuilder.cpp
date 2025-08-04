#include "ChunkMeshBuilder.h"
#include "Chunk.h"
#include "Logger.h"
#include "PathProvider.h"
#include "ScopedTimer.h"
#include "ServiceLocator.h"
#include "TextureManager.h"
#include "BlockCache.h"
#include "BlockFace.h"

#include <glm/glm.hpp>

static const std::array<std::string,6> faceKeys = {"east","west","up","down","south","north"};

ChunkMeshBuilder::ChunkMeshBuilder(Chunk& chunk) : chunk(chunk) {}

void ChunkMeshBuilder::buildMesh() {
    clear();
    chunk.updateChunkBlocksOpaqueData();
    auto& opaque = *chunk.getBlocksOpaqueData();
    const int s = Chunk::CHUNK_SIZE;

    for (int x = 0; x < s; ++x) 
    for (int y = 0; y < s; ++y) 
        for (int z = 0; z < s; ++z) {
            glm::ivec3 localPos(x, y, z);

            auto& block = *chunk.getBlocks()[chunk.toIndex(BlockPos(localPos))];
            Blocks type = block.getBlockId();
            const BlockModel& model = BlockCache::getInstance().getBlockModel(type);

            bool isOpaque = getOpaqueSafe(localPos);

            glm::ivec3 baseWorldPos = chunk.getChunkPos().position * s + localPos;

            if (!isOpaque) {
                for (int i = 0; i < 6; ++i) {
                    const std::string& key = faceKeys[i];
                    for (auto& element : model.elements) {
                        if (element.faces.find(key) == element.faces.end()) continue;
                        processBlockFace(glm::vec3(baseWorldPos), model, element, key);
                    }
                }
            } else {
                for (int i = 0; i < 6; ++i) {
                    const std::string& key = faceKeys[i];
                    const Face& face = faces[i];
                    glm::ivec3 neighbor = localPos + face.neighborOffset;
                    if (getOpaqueSafe(neighbor)) continue;

                    for (auto& element : model.elements) {
                        if (element.faces.find(key) == element.faces.end()) continue;
                        processBlockFace(glm::vec3(baseWorldPos), model, element, key);
                    }
                }
            }
        }

}

bool ChunkMeshBuilder::getOpaqueSafe(glm::ivec3 localPos) {
    auto* opaque = chunk.getBlocksOpaqueData();
    const int s = Chunk::CHUNK_SIZE;
    if (localPos.x>=0&&localPos.x<s&&localPos.y>=0&&localPos.y<s&&localPos.z>=0&&localPos.z<s) {
        return opaque->isOpaque(localPos.x, localPos.y, localPos.z);
    }
    glm::ivec3 offset(0);
    for (int i=0;i<3;++i) {
        if (localPos[i]<0) { offset[i]=-1; localPos[i]+=s; }
        else if (localPos[i]>=s) { offset[i]=1; localPos[i]-=s; }
    }
    ChunkPos np = chunk.getChunkPos(); np.position += offset;
    auto opt = ServiceLocator::GetWorld()->getChunkController().getChunk(np);
    if (!opt.has_value()) return false;
    return BlockCache::getInstance().getBlockInfo(opt->get().getBlock(BlockPos(localPos)).getBlockId()).isOpaque;
}

void ChunkMeshBuilder::processBlockFace(const glm::ivec3& worldBlockPos,
                                        const BlockModel& model,
                                        const BlockModelElement& element,
                                        const std::string& faceKey) {
    const BlockModelFace& faceData = element.faces.at(faceKey);
    
    std::string texKey = faceData.texture;
    if (!texKey.empty() && texKey[0] == '#') {
        texKey = texKey.substr(1);
    }

    auto it = model.textures.find(texKey);
    if (it == model.textures.end()) {
        Logger::getInstance().Log("Texture key not found: " + texKey, LogLevel::Warning);
        return;
    }
    std::string texturePath = it->second;

    const auto& atlasRegions = TextureManager::getInstance().getAtlasRegions();
    auto regionIt = atlasRegions.find(texturePath + ".png");
    if (regionIt == atlasRegions.end()) {
        Logger::getInstance().Log("Texture not found in atlas: " + texturePath, LogLevel::Warning);
        return;
    }
    const AtlasRegion& region = regionIt->second;

    glm::vec2 localUVs[4] = {
        { faceData.uv[0], faceData.uv[1] },
        { faceData.uv[2], faceData.uv[1] },
        { faceData.uv[2], faceData.uv[3] },
        { faceData.uv[0], faceData.uv[3] }
    };

    float uRange = region.u1 - region.u0;
    float vRange = region.v1 - region.v0;

    glm::vec2 quadUVs[4];
    for (int i = 0; i < 4; ++i) {
        glm::vec2 normUV = localUVs[i] / 16.0f;
        quadUVs[i] = { region.u0 + normUV.x * uRange,
                       region.v0 + normUV.y * vRange };
    }

    glm::vec3 verts[4];
    getFaceVertices(element, faceKey, verts);
    for (int i = 0; i < 4; ++i) {
        verts[i] += worldBlockPos;
    }

    glm::vec3 normal = getNormalForFace(faceKey);
    addQuad(verts, normal, quadUVs);
}

glm::vec3 ChunkMeshBuilder::getNormalForFace(const std::string& faceKey) {
    if (faceKey=="north") return {0,0,-1};
    if (faceKey=="south") return {0,0,1};
    if (faceKey=="west")  return {-1,0,0};
    if (faceKey=="east")  return {1,0,0};
    if (faceKey=="up")    return {0,1,0};
    return {0,-1,0};
}

void ChunkMeshBuilder::getFaceVertices(const BlockModelElement& e,
                                       const std::string& faceKey,
                                       glm::vec3 out[4]) {
    glm::vec3 f = glm::vec3(e.from[0], e.from[1], e.from[2]) / 16.0f;
    glm::vec3 t = glm::vec3(e.to[0],   e.to[1],   e.to[2])   / 16.0f;
    if (faceKey=="north") {
        out[0] = {f.x, f.y, f.z};
        out[1] = {t.x, f.y, f.z};
        out[2] = {t.x, t.y, f.z};
        out[3] = {f.x, t.y, f.z};
    } else if (faceKey=="south") {
        out[0] = {t.x, f.y, t.z};
        out[1] = {f.x, f.y, t.z};
        out[2] = {f.x, t.y, t.z};
        out[3] = {t.x, t.y, t.z};
    } else if (faceKey=="west") {
        out[0] = {f.x, f.y, t.z};
        out[1] = {f.x, f.y, f.z};
        out[2] = {f.x, t.y, f.z};
        out[3] = {f.x, t.y, t.z};
    } else if (faceKey=="east") {
        out[0] = {t.x, f.y, f.z};
        out[1] = {t.x, f.y, t.z};
        out[2] = {t.x, t.y, t.z};
        out[3] = {t.x, t.y, f.z};
    } else if (faceKey=="up") {
        out[0] = {f.x, t.y, f.z};
        out[1] = {t.x, t.y, f.z};
        out[2] = {t.x, t.y, t.z};
        out[3] = {f.x, t.y, t.z};
    } else { // down
        out[0] = {f.x, f.y, t.z};
        out[1] = {t.x, f.y, t.z};
        out[2] = {t.x, f.y, f.z};
        out[3] = {f.x, f.y, f.z};
    }
}

void ChunkMeshBuilder::addQuad(const glm::vec3* quadVerts,
                               const glm::vec3& normal,
                               const glm::vec2* quadUVs) {
    unsigned int start = vertices.size();
    for (int i=0;i<4;++i) {
        vertices.push_back(Vertex{quadVerts[i], normal, quadUVs[i]});
    }
    indices.insert(indices.end(), {start, start+2, start+1, start, start+3, start+2});
}

void ChunkMeshBuilder::clear() {
    vertices.clear(); indices.clear();
}

void ChunkMeshBuilder::update() { buildMesh(); }

size_t ChunkMeshBuilder::getVertexCount() const { return vertices.size(); }
size_t ChunkMeshBuilder::getIndexCount()  const { return indices.size(); }