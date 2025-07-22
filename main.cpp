#include <string>

#include "Logger.h"
#include "PathProvider.h"
#include "ThreadPool.h"
#include "EventBus.h"
#include "ServiceLocator.h"
#include "GlResourceDeleter.h"

#include "WindowController.h"
#include "Camera.h"
#include "Shader.h"

#include "StateController.h"
#include "MainMenuState.h"
#include "GameState.h"
#include "World.h"
#include "BlocksIncluder.h"
#include "RayCastHit.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "FontsLoader.h"
#include "TextureController.h"

#include "WireFrameCube.h"
#include "SkySettings.h"
#include "f3InfoScreen.h"

float deltaTime = 0.0f;
float lastFrame = 0.0f;
float timeOfDay = 0;
glm::vec3 sunDirection = glm::normalize(glm::vec3(-0.5f, -1.0f, -0.3f));
std::optional<RaycastHit> raycastHit;

// Later create an entity class - and then Player will be an entity and will have a camera and choosed block controls
Camera camera;
Blocks choosedBlock = Blocks::Stone;

void init() {
    Logger::getInstance().Log("Application started", LogLevel::Info, LogOutput::Both, LogWriteMode::Overwrite);
    ThreadPool::getInstance(); // Initialize ThreadPool
    EventBus::getInstance(); // Initialize EventBus
    FileHandler::getInstance(); // Initialize FileHandler
}

glm::vec3 interpolateColor(const glm::vec3& dayColor, const glm::vec3& nightColor, float time);

int main() {
    init();

    WindowController windowController;
    windowController.init();
    //windowController.toggleFullscreen();
    windowController.bindCameraControls(&camera);
    windowController.setWindowTitle("MineOx 0.2");
    GLFWwindow* window = windowController.getWindow();
    FontsLoader fontsLoader;
    f3InfoScreen f3InfoScreen;

    if (!window) {
        Logger::getInstance().Log("Failed to create GLFW window", LogLevel::Critical, LogOutput::Both);
        return -1;
    }

    StateController stateController;
    stateController.changeState(std::make_unique<MainMenuState>());

    camera = Camera();
    World world(12345, "New world"); // Later, get seed and world name from user input
    ServiceLocator::ProvideWorld(&world);
    
    if (!FileHandler::getInstance().fileExists(PathProvider::getInstance().getWorldChunksPath(world.getWorldName()))) {
        FileHandler::getInstance().createDirectory(PathProvider::getInstance().getWorldChunksPath(world.getWorldName()));
    }
    if (!FileHandler::getInstance().fileExists(PathProvider::getInstance().getDataPath())) {
        FileHandler::getInstance().createDirectory(PathProvider::getInstance().getDataPath());
    }
    if (!FileHandler::getInstance().fileExists(PathProvider::getInstance().getTextureFolderPath())) {
        FileHandler::getInstance().createDirectory(PathProvider::getInstance().getTextureFolderPath());
    }
    if (!FileHandler::getInstance().fileExists(PathProvider::getInstance().getBlocksTextureFolderPath())) {
        FileHandler::getInstance().createDirectory(PathProvider::getInstance().getBlocksTextureFolderPath());
    }
    //world.init(PlayerPos(glm::ivec3(camera.Position.x, camera.Position.y, camera.Position.z)));

    std::string mainShader1 = PathProvider::getInstance().getMainShadersPath()[0].string();
    std::string mainShader2 = PathProvider::getInstance().getMainShadersPath()[1].string();
    std::string wireFrameCubeShaderPath1 = PathProvider::getInstance().getWireFrameCubeShadersPath()[0].string();
    std::string wireFrameCubeShaderPath2 = PathProvider::getInstance().getWireFrameCubeShadersPath()[1].string();
    std::string fontsShaderPath1 = PathProvider::getInstance().getFontShadersPath()[0].string();
    std::string fontsShaderPath2 = PathProvider::getInstance().getFontShadersPath()[1].string();

    Shader shader(mainShader1.c_str(), mainShader2.c_str());
    Shader wireFrameCubeShader(wireFrameCubeShaderPath1.c_str(), wireFrameCubeShaderPath2.c_str());
    Shader fontsShader(fontsShaderPath1.c_str(), fontsShaderPath2.c_str());
    
    TextureController::getInstance().initialize(PathProvider::getInstance().getBlocksTextureFolderPath());
    TextureController::getInstance().initializeTextures(shader.ID);
    
    fontsLoader.loadFont((PathProvider::getInstance().getFontsPath() / "arial.ttf").string());
    fontsLoader.loadCharacters(32);

    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(WireFrameCube::vertices), WireFrameCube::vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(WireFrameCube::indices), WireFrameCube::indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    
    //glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
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

    glm::mat4 fontProjection = glm::ortho(0.0f, windowController.getWidth(), 0.0f,  windowController.getHeight());

    while (!glfwWindowShouldClose(window)) {
        if(windowController.shouldToggleFullscreen) {
            windowController.toggleFullscreen();
        } else if(window) {
            float currentFrame = static_cast<float>(glfwGetTime());
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;
            
            timeOfDay += deltaTime / SkySettings::DAY_LENGTH;
            if (timeOfDay > 1.0f)
                timeOfDay -= 1.0f;
            float sunAngle = timeOfDay * 2.0f * glm::pi<float>();

            // Рассчитать позицию солнца и направление
            glm::vec3 sunPosition = glm::vec3(glm::cos(sunAngle), glm::sin(sunAngle), glm::sin(sunAngle * 0.5f));
            glm::vec3 sunDirection = glm::normalize(-sunPosition);
            glm::vec3 sunColor = glm::vec3(1.0f, 0.95f, 0.8f);

            glm::vec3 currentSkyColor = interpolateColor(SkySettings::DAY_COLOR, SkySettings::NIGHT_COLOR, timeOfDay);

            glClearColor(currentSkyColor.r, currentSkyColor.g, currentSkyColor.b, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            //stateController.handleInput(window);
            //stateController.update(deltaTime);
            raycastHit = world.raycast(camera.Position, camera.Front, 5.0f);

            f3InfoScreen.fps = static_cast<int>(1.0f / deltaTime);
            f3InfoScreen.playerPos = glm::ivec3(camera.Position);
            f3InfoScreen.chunkPos = world.getChunkController().toChunkPos(glm::ivec3(camera.Position)).position;
            f3InfoScreen.blockPos = glm::ivec3(glm::ivec3(camera.Position));
            BlockPos blockPos = raycastHit.has_value() ? BlockPos(raycastHit->blockPos) : BlockPos(glm::ivec3(0));
            auto block = world.getChunkController().getBlock(blockPos);
            if(block) {
                f3InfoScreen.facedBlockInfo = raycastHit.has_value() 
                ? "Block: " + block.value().get().getStringRepresentation() + 
                  ", Face Normal: " + std::to_string(raycastHit->faceNormal.x) + 
                  ", " + std::to_string(raycastHit->faceNormal.y) + 
                  ", " + std::to_string(raycastHit->faceNormal.z)
                : "No block hit";
            } else {
                f3InfoScreen.facedBlockInfo = "No block hit";
            }
            
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
                wireFrameCubeShader.setVec3("color", glm::vec3(0.0f, 1.0f, 0.0f));

                glLineWidth(2.0f);

                glBindVertexArray(VAO);
                glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);
            }

            //stateController.render();
            float wHeight = static_cast<float>(windowController.getHeight());
            glDisable(GL_DEPTH_TEST);
            fontsLoader.RenderText(fontsShader, "FPS: " + f3InfoScreen.toString(f3InfoScreen.fps), 10.0f, wHeight - 30.0f, 1.0f, glm::vec3(0.0, 0.0f, 0.0f), fontProjection);
            fontsLoader.RenderText(fontsShader, f3InfoScreen.toString(f3InfoScreen.blockPos), 10.0f, wHeight - 60.0f, 1.0f, glm::vec3(0.0, 0.0f, 0.0f), fontProjection);
            fontsLoader.RenderText(fontsShader, f3InfoScreen.toString(f3InfoScreen.chunkPos), 10.0f, wHeight - 90.0f, 1.0f, glm::vec3(0.0, 0.0f, 0.0f), fontProjection);
            fontsLoader.RenderText(fontsShader, f3InfoScreen.toString(f3InfoScreen.playerPos), 10.0f, wHeight - 120.0f, 1.0f, glm::vec3(0.0, 0.0f, 0.0f), fontProjection);
            fontsLoader.RenderText(fontsShader, f3InfoScreen.facedBlockInfo, 10.0f, wHeight - 150.0f, 1.0f, glm::vec3(0.0, 0.0f, 0.0f), fontProjection);
            glEnable(GL_DEPTH_TEST);

            if(windowController.shouldTakeScreenshot) {
                windowController.doTheScreenshotAndSave();
                Logger::getInstance().Log("Screenshot taken", LogLevel::Info, LogOutput::Both, LogWriteMode::Append);
                windowController.shouldTakeScreenshot = false;
            }

            GLResourceDeleter::getInstance().processDeletes();
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }


    Logger::getInstance().Log("Application shutdown", LogLevel::Warning, LogOutput::Both, LogWriteMode::Append);
    windowController.shutdown();
    return 0;
}

glm::vec3 interpolateColor(const glm::vec3& dayColor, const glm::vec3& nightColor, float time) {
    float t = (time <= 0.5f) ? (time * 2.0f) : ((1.0f - time) * 2.0f);
    return glm::mix(dayColor, dayColor, t);
}