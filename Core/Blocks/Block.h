#pragma once

#include <string>

#include "Blocks.h"

class Block {
public:
    virtual void onPlace() = 0;
    virtual void onBreak() = 0;
    virtual void onUpdate(float deltaTime) = 0;
    virtual void render() = 0;
    virtual void onInteract() {}
    virtual void onNeighborChanged() {}

    virtual Blocks getBlockId() const = 0;

    virtual std::string getBlockProperties() const = 0;
    virtual void setBlockProperties(const std::string& properties) = 0;

    virtual std::array<std::string, 6> getBlockSidesTextureNames() = 0;

    virtual ~Block() = default;
};