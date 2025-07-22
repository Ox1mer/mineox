#pragma once

struct WireFrameCube
{
    static constexpr float vertices[] = {
        -0.5f, -0.5f, -0.5f, // 0
        0.5f, -0.5f, -0.5f, // 1
        0.5f,  0.5f, -0.5f, // 2
        -0.5f,  0.5f, -0.5f, // 3
        -0.5f, -0.5f,  0.5f, // 4
        0.5f, -0.5f,  0.5f, // 5
        0.5f,  0.5f,  0.5f, // 6
        -0.5f,  0.5f,  0.5f  // 7
    };

    static constexpr unsigned int indices[] = {
        // Back edge
        0, 1,
        1, 2,
        2, 3,
        3, 0,

        // Front edge
        4, 5,
        5, 6,
        6, 7,
        7, 4,

        // Side edges
        0, 4,
        1, 5,
        2, 6,
        3, 7
    };
};
