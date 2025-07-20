#pragma once

#include <memory>
#include "World.h"

class World;

class ServiceLocator {
private:
    inline static World* worldInstance = nullptr;
public:
    ServiceLocator() = delete;

    static void ProvideWorld(World* world) {
        worldInstance = world;
    }

    static World* GetWorld() {
        return worldInstance;
    }
};
