#pragma once
#include "Block.h"

class MigmatiteBlock : public Block {
public:
    void onPlace() override {
        
    }

    void onBreak() override {
        
    }

    void onUpdate(float deltaTime) override {
        
    }

    void render() override {
        
    }

    std::string getStringRepresentation() const override {
        return "migmatite";
    }

    Blocks getBlockId() const override {
        return Blocks::Migmatite;
    }

    std::string getBlockProperties() const override {
        return "{}";
    }

    void setBlockProperties(const std::string& properties) override {
        
    }

    bool isTransparent() const override {
        return false;
    }

    bool isSolid() const override {
        return true;
    }

    std::array<std::string, 6> getBlockSidesTextureNames() override {
        return {
            "migmatite.png",
            "migmatite.png",
            "migmatite.png",
            "migmatite.png",
            "migmatite.png",
            "migmatite.png"
        };
    }
};