#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>
#include <glad/glad.h>
#include <stb_image.h>

#include "Vertex.h"
#include "BlockFace.h"
#include "PathProvider.h"
#include "ChunkBlocksOpaqueData.h"
#include "BlockPos.h"

class Chunk;
struct AtlasRegion;

class ChunkMeshBuilder {
public:
    explicit ChunkMeshBuilder(Chunk& chunk);

    void buildMesh();
    void update();

    void clear();

    size_t getVertexCount() const;
    size_t getIndexCount() const;

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

private:
    bool getOpaqueSafe(ChunkBlocksOpaqueData* opaque, glm::ivec3 pos);
    void addQuad(glm::ivec3 pos,
                               glm::ivec3 u,
                               glm::ivec3 v,
                               const Face& face,
                               int w,
                               int h,
                               const AtlasRegion region);
    GLuint loadTexture(const fs::path& path);
    Chunk& chunk;
    std::unordered_map<std::string, int> textureIDs;
    int i = 0;
};
