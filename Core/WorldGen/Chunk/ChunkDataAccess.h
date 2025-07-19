#pragma once

#include <string>
#include <sstream>

#include "Chunk.h"
#include "ChunkPos.h"
#include "FileHandler.h"
#include "PathProvider.h"
#include "ScopedTimer.h"

class ChunkDataAccess {
public:
    bool saveChunkToDisk(const ChunkPos& pos, const Chunk& chunk, const std::string& worldName) {
        ScopedTimer timer("saveChunkToFile");

        auto& fileHandler = FileHandler::getInstance();
        fs::path chunkFilePath = PathProvider::getInstance().getChunkFilePath(worldName, pos);

        std::stringstream ss;

        // --- File version ---
        ss << CHUNK_FILE_VERSION << '\n';

        // --- Count of blocks ---
        size_t size = chunk.getBlocks().size();
        ss << size << '\n';

        for (const auto& b : chunk.getBlocks()) {
            Blocks id = b->getBlockId();

            if (id == Blocks::Air) {
                ss << 0 << '\n';
                continue;
            }

            ss << static_cast<int>(id) << ' ';

            std::string props = b->getBlockProperties();
            ss << props.size() << ' ' << props << '\n';
        }

        std::string data = ss.str();
        fileHandler.writeToFile(chunkFilePath, data);

        return true;
    }

    std::optional<std::unique_ptr<Chunk>> loadChunkFromDisk(const ChunkPos& pos, const std::string& worldName) {
        auto& fileHandler = FileHandler::getInstance();
        fs::path chunkFilePath = PathProvider::getInstance().getChunkFilePath(worldName, pos);

        if (!fileHandler.fileExists(chunkFilePath)) {
            return std::nullopt;
        }
        std::string data;

        fileHandler.readFromFile(chunkFilePath, data);

        std::istringstream iss(data);
        uint32_t version = 0;
        iss >> version;
        if (version != CHUNK_FILE_VERSION) {
            throw std::runtime_error("Chunk file version mismatch!");
        }

        size_t blocksCount = 0;
        iss >> blocksCount;
        iss.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        auto chunk = std::make_unique<Chunk>(pos);

        std::vector<std::pair<BlockPos, Blocks>> blockChanges;
        for (size_t i = 0; i < blocksCount; ++i) {
            int idInt = 0;
            iss >> idInt;

            if (idInt == 0) {
                iss.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }

            size_t propsSize = 0;
            iss >> propsSize;
            iss.get();

            std::string props(propsSize, '\0');
            iss.read(&props[0], propsSize);
            iss.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            int x = i % Chunk::CHUNK_SIZE;
            int y = (i / Chunk::CHUNK_SIZE) % Chunk::CHUNK_SIZE;
            int z = i / (Chunk::CHUNK_SIZE * Chunk::CHUNK_SIZE);

            BlockPos blockPos{ glm::ivec3(x, y, z) };
            Blocks id = static_cast<Blocks>(idInt);

            blockChanges.emplace_back(blockPos, id);
        }
        chunk->setBlocks(blockChanges);
        return chunk;
    }

private:
    static constexpr uint32_t CHUNK_FILE_VERSION = 1;
};
