#pragma once
#include "Block.h"

class SporeMossBlock : public Block {
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
        return Blocks::SporeMoss;
    }

    std::string getBlockProperties() const override {
        return "{}";
    }

    void setBlockProperties(const std::string& properties) override {
        // Spore Moss block has no properties to set
    }

    std::array<std::string, 6> getBlockSidesTextureNames() override {
        return {
            "spore_moss.png",
            "spore_moss.png",
            "spore_moss.png",
            "spore_moss.png",
            "spore_moss.png",
            "spore_moss.png"
        };
    }

};