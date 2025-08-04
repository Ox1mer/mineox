#pragma once

#include <string>

struct BlockInfo {
    std::string name;
    bool isTransparent;
    bool isSolid;
    bool isOpaque;
};