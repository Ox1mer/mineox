#pragma once
#include "Block.h"

class AirBlock : public Block {
public:
    void onPlace() override {
        // No specific action for placing air
    }

    void onBreak() override {
        // No specific action for breaking air
    }

    void onUpdate(float deltaTime) override {
        // Air block does not update
    }

    void render() override {
        // Air block is not rendered
    }

    std::string getStringRepresentation() const override {
        return "air";
    }

    Blocks getBlockId() const override {
        return Blocks::Air;
    }

    std::string getBlockProperties() const override {
        return "{}";
    }

    void setBlockProperties(const std::string& properties) override {
        
    }

    bool isTransparent() const override {
        return true;
    }

    bool isSolid() const override {
        return false;
    }

    std::array<std::string, 6> getBlockSidesTextureNames() override {
        return {"","","","","",""};
    }
};