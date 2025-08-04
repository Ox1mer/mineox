#pragma once

#include <array>
#include <optional>
#include "JsonBlockInfoReader.h"

class BlockCache {
public:
    static BlockCache& getInstance() {
        static BlockCache instance;
        return instance;
    }

    void loadAll() {
        JsonBlockInfoReader reader;

        for (int i = 0; i < BLOCK_COUNT; ++i) {
            auto block = static_cast<Blocks>(i);

            // Чтение модели
            auto modelOpt = reader.readBlockModelInfo(block);
            if (modelOpt.has_value()) {
                blockModels[i] = std::move(modelOpt.value());
            } else {
                blockModels[i] = getDefaultBlockModel();
            }

            // Чтение общей инфы
            auto infoOpt = reader.readBlockInfo(block);
            if (infoOpt.has_value()) {
                blockInfos[i] = std::move(infoOpt.value());
            } else {
                blockInfos[i] = getDefaultBlockInfo();
            }
        }
    }

    const BlockModel& getBlockModel(Blocks block) const {
        return blockModels[static_cast<int>(block)];
    }

    const BlockInfo& getBlockInfo(Blocks block) const {
        return blockInfos[static_cast<int>(block)];
    }

private:
    BlockCache() = default;

    static constexpr int BLOCK_COUNT = static_cast<int>(Blocks::Count);
    std::array<BlockModel, BLOCK_COUNT> blockModels;
    std::array<BlockInfo, BLOCK_COUNT> blockInfos;

    BlockModel getDefaultBlockModel() const {
        return BlockModel{};
    }

    BlockInfo getDefaultBlockInfo() const {
        return BlockInfo{};
    }
};