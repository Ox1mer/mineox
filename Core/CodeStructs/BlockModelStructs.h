#pragma once

#include <string>
#include <vector>
#include <map>

struct BlockModelRotation {
    float angle;
    std::string axis;
    std::vector<float> origin;
};

struct BlockModelFace {
    std::vector<float> uv;
    std::string texture;
};

struct BlockModelElement {
    std::vector<float> from;
    std::vector<float> to;
    BlockModelRotation rotation;
    std::map<std::string, BlockModelFace> faces;
};

struct BlockModel {
    std::string credit;
    std::map<std::string, std::string> textures;
    std::vector<BlockModelElement> elements;
};