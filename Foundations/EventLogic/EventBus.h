#pragma once

#include <functional>
#include <unordered_map>
#include <vector>
#include <shared_mutex>
#include <mutex>
#include <typeindex>
#include <atomic>
#include <algorithm>
#include <memory>
#include <cstdint>


class EventBus {
public:
    // Unique identifier for a subscription
    struct SubscriptionToken {
        std::type_index eventType;
        std::uint64_t id;
    };

    static EventBus& getInstance() {
        static EventBus instance;
        return instance;
    }

    // Subscribe to an event type. Returns token for unsubscription.
    template<typename EventT>
    SubscriptionToken subscribe(std::function<void(const EventT&)> handler) {
        auto type = std::type_index(typeid(EventT));
        std::uint64_t tokenId = nextId_++;
        {
            std::unique_lock<std::shared_mutex> lock(mutex_);
            auto& containerAny = handlers_[type];
            if (!containerAny) {
                containerAny = std::make_shared<HandlerContainer<EventT>>();
            }
            auto container = std::static_pointer_cast<HandlerContainer<EventT>>(containerAny);
            container->addHandler(tokenId, std::move(handler));
        }
        return {type, tokenId};
    }

    // Unsubscribe using token
    void unsubscribe(const SubscriptionToken& token) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        auto it = handlers_.find(token.eventType);
        if (it == handlers_.end() || !it->second) return;
        auto basePtr = it->second;
        unsubscribeHelper(basePtr, token.id);
    }

    // Emit event to all subscribers
    template<typename EventT>
    void emit(const EventT& event) {
        auto type = std::type_index(typeid(EventT));
        std::shared_ptr<HandlerContainer<EventT>> container;
        {
            std::shared_lock<std::shared_mutex> lock(mutex_);
            auto it = handlers_.find(type);
            if (it == handlers_.end() || !it->second) return;
            container = std::static_pointer_cast<HandlerContainer<EventT>>(it->second);
        }
        container->invoke(event);
    }

private:
    EventBus() = default;
    ~EventBus() = default;
    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;

    // Base class for type-erased handler container
    struct IHandlerContainer {
        virtual ~IHandlerContainer() = default;
        virtual void removeHandler(std::uint64_t /*id*/) = 0;
    };

    template<typename EventT>
    class HandlerContainer : public IHandlerContainer {
    public:
        void addHandler(std::uint64_t id, std::function<void(const EventT&)> handler) {
            std::unique_lock<std::shared_mutex> lock(containerMutex_);
            handlers_.emplace_back(id, std::move(handler));
        }

        void removeHandler(std::uint64_t id) override {
            std::unique_lock<std::shared_mutex> lock(containerMutex_);
            handlers_.erase(
                std::remove_if(handlers_.begin(), handlers_.end(),
                    [id](auto& p) { return p.first == id; }),
                handlers_.end());
        }

        void invoke(const EventT& event) {
            std::vector<std::function<void(const EventT&)>> toCall;
            {
                std::shared_lock<std::shared_mutex> lock(containerMutex_);
                toCall.reserve(handlers_.size());
                for (auto& p : handlers_) toCall.emplace_back(p.second);
            }
            for (auto& func : toCall) func(event);
        }

    private:
        std::vector<std::pair<std::uint64_t, std::function<void(const EventT&)>>> handlers_;
        mutable std::shared_mutex containerMutex_;
    };

    // Helper to unsubscribe without knowing EventT
    void unsubscribeHelper(std::shared_ptr<IHandlerContainer> basePtr, std::uint64_t id) {
        basePtr->removeHandler(id);
    }

    std::unordered_map<std::type_index, std::shared_ptr<IHandlerContainer>> handlers_;
    std::shared_mutex mutex_;
    std::atomic<std::uint64_t> nextId_{1};
};

// Usage example:
// EventBus::getInstance().subscribe<MyEventType>([](const MyEventType& event) {
//     // Handle event
// });

// EventBus::getInstance().unsubscribe(token);

//MyEvent evt{};
//EventBus::getInstance().emit(evt);