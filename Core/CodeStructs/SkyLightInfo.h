#pragma once

#include <glm/glm.hpp>

struct SkyLightInfo
{
    glm::vec3 lightDirection;
    glm::vec3 lightColor;
    glm::vec3 skyColor;
    float lightIntensity;
};
