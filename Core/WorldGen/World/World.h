#pragma once

#include <string>

#include "ChunkController.h"
#include "PlayerPos.h"
#include "Shader.h"

class World {
private:
    int seed;
    std::string worldName;
    ChunkController _chunkController;
    int viewDistance = 1;

public:
    World(int seed, std::string worldName)
        : seed(seed), worldName(worldName), _chunkController(worldName) {}

    ~World() {}

    void update(PlayerPos plPos) {
        _chunkController.update(plPos.position, viewDistance);
    }

    void render(Shader shader) {
        _chunkController.renderAllChunks(shader);
    }

    void save() {}

    ChunkController& getChunkController() { return _chunkController; }
    int getSeed() const { return seed; }
    const std::string& getWorldName() const { return worldName; }
};
