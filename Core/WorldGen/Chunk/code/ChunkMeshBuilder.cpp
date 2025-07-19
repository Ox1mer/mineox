#include "ChunkMeshBuilder.h"
#include "Chunk.h"
#include "Logger.h"
#include "PathProvider.h"
#include "ScopedTimer.h"

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

                        int textureID = TextureController::getInstance().getTextureID(blockName);

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

                        addQuad(worldPos, u, v, face, 1, 1, textureID);
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
                               int textureID) {
    static int addQuadCounter = 0;
    ++addQuadCounter;
    unsigned int startIndex = vertices.size();
    glm::vec3 normal = face.normal;

    glm::vec3 v0 = glm::vec3(pos);
    glm::vec3 v1 = v0 + glm::vec3(u) * float(w);
    glm::vec3 v2 = v1 + glm::vec3(v) * float(h);
    glm::vec3 v3 = v0 + glm::vec3(v) * float(h);

    vertices.push_back(Vertex{v0, normal, {0, 0}, textureID});
    vertices.push_back(Vertex{v1, normal, {1, 0}, textureID});
    vertices.push_back(Vertex{v2, normal, {1, 1}, textureID});
    vertices.push_back(Vertex{v3, normal, {0, 1}, textureID});

    indices.push_back(startIndex + 0);
    indices.push_back(startIndex + 1);
    indices.push_back(startIndex + 2);
    indices.push_back(startIndex + 2);
    indices.push_back(startIndex + 3);
    indices.push_back(startIndex + 0);
}

bool ChunkMeshBuilder::getOpaqueSafe(ChunkBlocksOpaqueData* opaque, glm::ivec3 localPos) {
    int s = Chunk::CHUNK_SIZE;
    if (localPos.x < 0 || localPos.y < 0 || localPos.z < 0 ||
        localPos.x >= s || localPos.y >= s || localPos.z >= s)
        return false;
    return opaque->isOpaque(localPos.x, localPos.y, localPos.z);
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

GLuint ChunkMeshBuilder::loadTexture(const fs::path& path) {
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.string().c_str(), &width, &height, &nrChannels, 0);

    if (!data) {
        std::cerr << "Failed to load texture: " << path << std::endl;
        return 0;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    GLenum format = (nrChannels == 1 ? GL_RED : (nrChannels == 3 ? GL_RGB : GL_RGBA));
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    return textureID;
}
