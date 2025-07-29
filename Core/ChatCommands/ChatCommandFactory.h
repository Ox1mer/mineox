#pragma once

#include <unordered_map>
#include <functional>
#include <memory>
#include <mutex>

#include "IChatCommand.h"
#include "ChatCommands.h"

struct ChatCommandIDHash {
    std::size_t operator()(const ChatCommandID& id) const {
        return static_cast<std::size_t>(id);
    }
};

class ChatCommandFactory {
public:
    using CommandConstructor = std::function<std::unique_ptr<IChatCommand>()>;

    static ChatCommandFactory& getInstance() {
        static ChatCommandFactory instance;
        return instance;
    }

    void registerCommand(ChatCommandID id, CommandConstructor constructor) {
        std::lock_guard<std::mutex> lock(_mutex);
        _registry[id] = constructor;
    }

    std::unique_ptr<IChatCommand> create(ChatCommandID id) {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _registry.find(id);
        if (it != _registry.end()) {
            return (it->second)();
        }
        return nullptr;
    }

    bool isRegistered(ChatCommandID id) {
        std::lock_guard<std::mutex> lock(_mutex);
        return _registry.count(id) > 0;
    }

private:
    ChatCommandFactory() = default;
    ~ChatCommandFactory() = default;

    ChatCommandFactory(const ChatCommandFactory&) = delete;
    ChatCommandFactory& operator=(const ChatCommandFactory&) = delete;

    std::unordered_map<ChatCommandID, CommandConstructor, ChatCommandIDHash> _registry;
    std::mutex _mutex;
};
