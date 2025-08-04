#pragma once
#include "Block.h"

class SandBlock : public Block {
public:
    void onPlace() override {
        // No specific action for placing dirt
    }

    void onBreak() override {
        // No specific action for breaking dirt
    }

    void onUpdate(float deltaTime) override {
        // Dirt block does not update
    }

    void render() override {
        // Render logic for dirt block
    }

    Blocks getBlockId() const override {
        return Blocks::Sand;
    }

    std::string getBlockProperties() const override {
        return "{}";
    }

    void setBlockProperties(const std::string& properties) override {
        // Sand block has no properties to set
    }

    std::array<std::string, 6> getBlockSidesTextureNames() override {
        return {
            "sand.png",
            "sand.png",
            "sand.png",
            "sand.png",
            "sand.png",
            "sand.png"
        };
    }
};