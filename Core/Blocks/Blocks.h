#pragma once

#include <string>

enum class Blocks {
    Air,
    Dirt,
    Gneiss,
    Gravel,
    Migmatite,
    Sand,
    SporeMoss,
    Stone,


    Count
};

inline std::string toString(Blocks block) {
    switch (block) {
        case Blocks::Air: return "air";
        case Blocks::Dirt: return "dirt";
        case Blocks::Gneiss: return "gneiss";
        case Blocks::Gravel: return "gravel";
        case Blocks::Migmatite: return "migmatite";
        case Blocks::Sand: return "sand";
        case Blocks::SporeMoss: return "spore_moss";
        case Blocks::Stone: return "stone";
        default: return "unknown";
    }
}