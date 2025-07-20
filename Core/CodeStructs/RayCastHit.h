#pragma once

#include <glm\glm.hpp>

struct RaycastHit {
    glm::ivec3 blockPos;
    glm::ivec3 faceNormal;
};