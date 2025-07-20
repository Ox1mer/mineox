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
#include "RayCastHit.h"
#include "ServiceLocator.h"

Camera camera;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
glm::vec3 sunDirection = glm::normalize(glm::vec3(-0.5f, -1.0f, -0.3f));
glm::vec3 sunColor = glm::vec3(1.0f, 1.0f, 0.9f);
float dayLength = 60.0f * 20.0f;
glm::vec3 daySkyColor = glm::vec3(0.53f, 0.81f, 0.92f);
glm::vec3 nightSkyColor = glm::vec3(0.0f, 0.0f, 0.05f);
float timeOfDay = 0;
std::optional<RaycastHit> raycastHit;
Blocks choosedBlock = Blocks::Dirt;

float cubeVertices[] = {
    -0.5f, -0.5f, -0.5f, // 0
     0.5f, -0.5f, -0.5f, // 1
     0.5f,  0.5f, -0.5f, // 2
    -0.5f,  0.5f, -0.5f, // 3
    -0.5f, -0.5f,  0.5f, // 4
     0.5f, -0.5f,  0.5f, // 5
     0.5f,  0.5f,  0.5f, // 6
    -0.5f,  0.5f,  0.5f  // 7
};

unsigned int cubeIndices[] = {
    // Задняя грань
    0, 1,
    1, 2,
    2, 3,
    3, 0,

    // Передняя грань
    4, 5,
    5, 6,
    6, 7,
    7, 4,

    // Боковые рёбра
    0, 4,
    1, 5,
    2, 6,
    3, 7
};

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
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

glm::vec3 interpolateColor(const glm::vec3& dayColor, const glm::vec3& nightColor, float time) {
    float t = (time <= 0.5f) ? (time * 2.0f) : ((1.0f - time) * 2.0f);
    return glm::mix(dayColor, dayColor, t);
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
    ServiceLocator::ProvideWorld(&world);
    if (!FileHandler::getInstance().fileExists(PathProvider::getInstance().getWorldChunksPath(world.getWorldName()))) {
        FileHandler::getInstance().createDirectory(PathProvider::getInstance().getWorldChunksPath(world.getWorldName()));
    }
    //world.init(PlayerPos(glm::ivec3(camera.Position.x, camera.Position.y, camera.Position.z)));

    Shader shader("C:\\Users\\Oximer\\Documents\\VSCodeProjects\\MineOx_0.2\\Shaders\\vertexShader.vs",
         "C:\\Users\\Oximer\\Documents\\VSCodeProjects\\MineOx_0.2\\Shaders\\fragmentShader.fs");
    
    Shader wireFrameCubeShader("C:\\Users\\Oximer\\Documents\\VSCodeProjects\\MineOx_0.2\\Shaders\\wireFrameCubeVertexShader.vs",
         "C:\\Users\\Oximer\\Documents\\VSCodeProjects\\MineOx_0.2\\Shaders\\wireFrameCubeFragmentShader.fs");
    
    TextureController::getInstance().initialize(PathProvider::getInstance().getBlocksTextureFolderPath());
    InitializeTextures(shader.ID);
    
    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glm::mat4 projection = glm::perspective(
        glm::radians(camera.FOV),
        (float)windowController.getWidth() / (float)windowController.getHeight(),
        0.1f,
        1000.0f
    );

    glm::mat4 model = glm::mat4(1.0f);
    timeOfDay = 0.25f;
    while (!glfwWindowShouldClose(window)) {
        if(windowController.shouldToggleFullscreen) {
            windowController.toggleFullscreen();
        } else if(window) {
            float currentFrame = static_cast<float>(glfwGetTime());
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;
            
            timeOfDay += deltaTime / dayLength;
            if (timeOfDay > 1.0f)
                timeOfDay -= 1.0f;
            float sunAngle = timeOfDay * 2.0f * glm::pi<float>();

            // Рассчитать позицию солнца и направление
            glm::vec3 sunPosition = glm::vec3(glm::cos(sunAngle), glm::sin(sunAngle), glm::sin(sunAngle * 0.5f));
            glm::vec3 sunDirection = glm::normalize(-sunPosition);
            glm::vec3 sunColor = glm::vec3(1.0f, 0.95f, 0.8f);

            glfwPollEvents();

            glm::vec3 currentSkyColor = interpolateColor(daySkyColor, nightSkyColor, timeOfDay);

            glClearColor(currentSkyColor.r, currentSkyColor.g, currentSkyColor.b, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            //stateController.handleInput(window);
            //stateController.update(deltaTime);
            raycastHit = world.raycast(camera.Position, camera.Front, 5.0f);
            windowController.processCameraKeyboard(&camera, deltaTime, raycastHit, choosedBlock);
            world.update(PlayerPos(glm::ivec3(camera.Position.x, camera.Position.y, camera.Position.z)));

            shader.use();
            shader.setMat4("view", camera.GetViewMatrix());
            shader.setMat4("projection", projection);
            shader.setMat4("model", model);

            world.render(shader, sunDirection, sunColor);

            if (raycastHit) {
                auto hit = *raycastHit;

                glm::mat4 wfCubemodel = glm::mat4(1.0f);
                wfCubemodel = glm::translate(wfCubemodel, glm::vec3(hit.blockPos) + glm::vec3(0.5f));
                wfCubemodel = glm::scale(wfCubemodel, glm::vec3(1.01f));

                wireFrameCubeShader.use();
                wireFrameCubeShader.setMat4("model", wfCubemodel);
                wireFrameCubeShader.setMat4("view", camera.GetViewMatrix());
                wireFrameCubeShader.setMat4("projection", projection);

                glLineWidth(2.0f);

                glBindVertexArray(VAO);
                glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);
            }

            //stateController.render();

            GLResourceDeleter::getInstance().processDeletes();
            glfwSwapBuffers(window);
        }
    }


    Logger::getInstance().Log("Application shutdown", LogLevel::Warning, LogOutput::Both, LogWriteMode::Append);
    windowController.shutdown();
    return 0;
}