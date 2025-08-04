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
#include "BlockModelStructs.h"

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
    const std::vector<Vertex>& getVertices() const { return vertices; }
    const std::vector<unsigned int>& getIndices() const { return indices; }

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

private:
    Chunk& chunk;

    bool getOpaqueSafe(glm::ivec3 localPos);
    void processBlockFace(const glm::ivec3& worldBlockPos,
                          const BlockModel& model,
                          const BlockModelElement& element,
                          const std::string& faceKey);
    void addQuad(const glm::vec3* quadVerts,
                 const glm::vec3& normal,
                 const glm::vec2* quadUVs);
    glm::vec3 getNormalForFace(const std::string& faceKey);
    void getFaceVertices(const BlockModelElement& element,
                         const std::string& faceKey,
                         glm::vec3 outVerts[4]);
};