#pragma once

#include <glm/glm.hpp>

struct SkySettings {
    constexpr static glm::vec3 DAY_COLOR = {0.53f, 0.81f, 0.92f};
    constexpr static glm::vec3 NIGHT_COLOR = {0.0f, 0.0f, 0.05f};
    constexpr static glm::vec3 SUN_COLOR = {1.0f, 1.0f, 1.0f};
    constexpr static float DAY_LENGTH = 60.0f * 20.0f;
};