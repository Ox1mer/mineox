#pragma once

#include <unordered_map>
#include <string>
#include <functional>
#include <memory>
#include <mutex>

#include "Block.h"
#include "Logger.h"

struct BlockIDHash {
    std::size_t operator()(const Blocks& id) const {
        return static_cast<std::size_t>(id);
    }
};

class BlockFactory {
public:
    using BlockConstructor = std::function<std::shared_ptr<Block>()>;

    static BlockFactory& getInstance() {
        static BlockFactory instance;
        return instance;
    }

    void registerBlock(Blocks blockType, BlockConstructor constructor) {
        std::lock_guard<std::mutex> lock(_mutex);
        _registry[blockType] = std::move(constructor);
    }

    std::shared_ptr<Block> create(Blocks blockType) {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _registry.find(blockType);
        if (it != _registry.end()) {
            return (it->second)();
        }
        std::string msg = "Block ID not found: " + std::to_string(static_cast<uint16_t>(blockType));
        Logger::getInstance().Log(msg, LogLevel::Error, LogOutput::Both, LogWriteMode::Overwrite);
        throw std::runtime_error(msg);
        return nullptr;
    }

    bool isRegistered(Blocks blockType) {
        std::lock_guard<std::mutex> lock(_mutex);
        return _registry.count(blockType) > 0;
    }

    std::shared_ptr<Block> getSharedAirBlock() {
        static std::shared_ptr<Block> airBlock = create(Blocks::Air);
        return airBlock;
    }

private:
    BlockFactory() = default;
    ~BlockFactory() = default;

    BlockFactory(const BlockFactory&) = delete;
    BlockFactory& operator=(const BlockFactory&) = delete;

    std::unordered_map<Blocks, BlockConstructor, BlockIDHash> _registry;
    std::mutex _mutex;
};