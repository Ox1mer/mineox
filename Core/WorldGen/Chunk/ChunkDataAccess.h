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
        auto& fileHandler = FileHandler::getInstance();
        fs::path chunkFilePath = PathProvider::getInstance().getChunkFilePath(worldName, pos);

        const auto& blocks = chunk.getBlocks();

        bool allAir = std::all_of(blocks.begin(), blocks.end(), [](const auto& b) {
            return b->getBlockId() == Blocks::Air;
        });

        std::ofstream ofs(chunkFilePath, std::ios::binary | std::ios::trunc);
        if (!ofs.is_open()) return false;

        if (allAir) {
            uint8_t zero = 0;
            ofs.write(reinterpret_cast<const char*>(&zero), sizeof(zero));
            return true;
        }

        uint32_t version = CHUNK_FILE_VERSION;
        ofs.write(reinterpret_cast<const char*>(&version), sizeof(version));

        uint32_t blocksCount = static_cast<uint32_t>(blocks.size());
        ofs.write(reinterpret_cast<const char*>(&blocksCount), sizeof(blocksCount));

        for (const auto& b : blocks) {
            Blocks id = b->getBlockId();
            uint32_t idInt = static_cast<uint32_t>(id);
            ofs.write(reinterpret_cast<const char*>(&idInt), sizeof(idInt));

            // air does`nt have params
            if (id == Blocks::Air) {
                continue;
            }

            std::string props = b->getBlockProperties();
            uint32_t propsSize = static_cast<uint32_t>(props.size());
            ofs.write(reinterpret_cast<const char*>(&propsSize), sizeof(propsSize));

            if (propsSize > 0) {
                ofs.write(props.data(), propsSize);
            }
        }

        ofs.close();
        return true;
    }


    std::optional<std::unique_ptr<Chunk>> loadChunkFromDisk(const ChunkPos& pos, const std::string& worldName) {
        auto& fileHandler = FileHandler::getInstance();
        fs::path chunkFilePath = PathProvider::getInstance().getChunkFilePath(worldName, pos);

        std::ifstream ifs(chunkFilePath, std::ios::binary);
        if (!ifs.is_open()) return std::nullopt;

        char firstByte = 0;
        ifs.read(&firstByte, 1);
        if (firstByte == 0) {
            return std::make_unique<Chunk>(pos);
        }

        ifs.seekg(0);
        uint32_t version = 0;
        ifs.read(reinterpret_cast<char*>(&version), sizeof(version));
        if (version != CHUNK_FILE_VERSION) throw std::runtime_error("Chunk file version mismatch!");

        uint32_t blocksCount = 0;
        ifs.read(reinterpret_cast<char*>(&blocksCount), sizeof(blocksCount));

        auto chunk = std::make_unique<Chunk>(pos);
        std::vector<std::pair<BlockPos, Blocks>> blockChanges;

        for (uint32_t i = 0; i < blocksCount; ++i) {
            uint32_t idInt = 0;
            ifs.read(reinterpret_cast<char*>(&idInt), sizeof(idInt));
            Blocks id = static_cast<Blocks>(idInt);

            std::string props;

            if (id != Blocks::Air) {
                uint32_t propsSize = 0;
                ifs.read(reinterpret_cast<char*>(&propsSize), sizeof(propsSize));
                if (propsSize > 0) {
                    props.resize(propsSize);
                    ifs.read(&props[0], propsSize);
                }
            }

            int x = i % Chunk::CHUNK_SIZE;
            int y = (i / Chunk::CHUNK_SIZE) % Chunk::CHUNK_SIZE;
            int z = i / (Chunk::CHUNK_SIZE * Chunk::CHUNK_SIZE);
            BlockPos blockPos{ glm::ivec3(x, y, z) };

            blockChanges.emplace_back(blockPos, id);

            chunk->getBlockPtr(blockPos)->setBlockProperties(props);
        }

        chunk->setBlocks(blockChanges);
        return chunk;
    }

private:
    static constexpr uint32_t CHUNK_FILE_VERSION = 1;
};
