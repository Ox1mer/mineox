#include <iostream>
#include <string>
#include "Logger.h"
#include "PathProvider.h"
#include "ThreadPool.h"
#include "EventBus.h"
#include "WindowController.h"
#include "StateController.h"
#include "MainMenuState.h"
#include "GameState.h"
#include "BlocksIncluder.h"
#include "World.h"
#include "Shader.h"
#include "Camera.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "GlResourceDeleter.h"
#include "TextureController.h"

Camera camera;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

void init() {
    Logger::getInstance().Log("Application started", LogLevel::Info, LogOutput::Both, LogWriteMode::Overwrite);
    ThreadPool::getInstance(); // Initialize ThreadPool
    EventBus::getInstance(); // Initialize EventBus
}

void InitializeTextures(GLuint shaderProgramID) {
    auto textureIDs = TextureController::getInstance().getAllTextureIDs();

    // Активируем и привязываем все текстуры
    for (GLuint i = 0; i < textureIDs.size(); ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, textureIDs[i]);
    }

    // Передаем массив индексов сэмплеров в шейдер
    GLint location = glGetUniformLocation(shaderProgramID, "textures");
    if (location == -1) {
        std::cerr << "Warning: uniform 'textures' not found in shader\n";
        return;
    }

    std::vector<GLint> samplers(textureIDs.size());
    for (int i = 0; i < samplers.size(); ++i)
        samplers[i] = i; // textures[0] => GL_TEXTURE0, textures[1] => GL_TEXTURE1, ...

    glUseProgram(shaderProgramID);
    glUniform1iv(location, samplers.size(), samplers.data());
    glUseProgram(0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

int main() {
    init();

    WindowController windowController;
    windowController.init();
    //windowController.toggleFullscreen();
    windowController.bindCameraControls(&camera);
    windowController.setWindowTitle("MineOx 0.2");
    GLFWwindow* window = windowController.getWindow();

    if (!window) {
        Logger::getInstance().Log("Failed to create GLFW window", LogLevel::Critical, LogOutput::Both);
        return -1;
    }

    StateController stateController;
    stateController.changeState(std::make_unique<MainMenuState>());

    camera = Camera();

    World world(12345, "New world");
    if (!FileHandler::getInstance().fileExists(PathProvider::getInstance().getWorldChunksPath(world.getWorldName()))) {
        FileHandler::getInstance().createDirectory(PathProvider::getInstance().getWorldChunksPath(world.getWorldName()));
    }
    //world.init(PlayerPos(glm::ivec3(camera.Position.x, camera.Position.y, camera.Position.z)));

    Shader shader("C:\\Users\\Oximer\\Documents\\VSCodeProjects\\MineOx_0.2\\Shaders\\vertexShader.vs",
         "C:\\Users\\Oximer\\Documents\\VSCodeProjects\\MineOx_0.2\\Shaders\\fragmentShader.fs");
    
    TextureController::getInstance().initialize(PathProvider::getInstance().getBlocksTextureFolderPath());
    InitializeTextures(shader.ID);
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_TEXTURE_2D);

    glm::mat4 projection = glm::perspective(
        glm::radians(camera.FOV),
        (float)windowController.getWidth() / (float)windowController.getHeight(),
        0.1f,
        1000.0f
    );

    glm::mat4 model = glm::mat4(1.0f);

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();

        glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //stateController.handleInput(window);
        //stateController.update(deltaTime);

        windowController.processCameraKeyboard(&camera, deltaTime);
        world.update(PlayerPos(glm::ivec3(camera.Position.x, camera.Position.y, camera.Position.z)));

        shader.use();
        shader.setMat4("view", camera.GetViewMatrix());
        shader.setMat4("projection", projection);
        shader.setMat4("model", model);

        world.render(shader);

        std::string coords = std::to_string(camera.Position.x) + " " +
                     std::to_string(camera.Position.y) + " " +
                     std::to_string(camera.Position.z);

        std::string rotation = std::to_string(camera.Pitch) + " " +
                     std::to_string(camera.Yaw);

        //stateController.render();

        GLResourceDeleter::getInstance().processDeletes();
        glfwSwapBuffers(window);
    }


    Logger::getInstance().Log("Application shutdown", LogLevel::Warning, LogOutput::Both, LogWriteMode::Append);
    windowController.shutdown();
    return 0;
}