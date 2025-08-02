#pragma once
#include "Block.h"

class GravelBlock : public Block {
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

    std::string getStringRepresentation() const override {
        return "gravel";
    }

    Blocks getBlockId() const override {
        return Blocks::Gravel;
    }

    std::string getBlockProperties() const override {
        return "{}";
    }

    void setBlockProperties(const std::string& properties) override {
        // Gravel block has no properties to set
    }

    bool isTransparent() const override {
        return false;
    }

    bool isSolid() const override {
        return true;
    }

    std::array<std::string, 6> getBlockSidesTextureNames() override {
        return {
            "gravel.png",
            "gravel.png",
            "gravel.png",
            "gravel.png",
            "gravel.png",
            "gravel.png"
        };
    }

};