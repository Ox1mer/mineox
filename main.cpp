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
#include <GL/glext.h>

float deltaTime = 0.0f;
float lastFrame = 0.0f;
std::optional<RaycastHit> raycastHit;

// Later create an entity class - and then Player will be an entity and will have a camera and choosed block controls
Camera camera;
Blocks choosedBlock = Blocks::Dirt;

void init() {
    Logger::getInstance().Log("Application started", LogLevel::Info, LogOutput::Both, LogWriteMode::Overwrite);
    ThreadPool::getInstance(); // Initialize ThreadPool
    EventBus::getInstance(); // Initialize EventBus
    FileHandler::getInstance(); // Initialize FileHandler
}

int main() {
    init();

    WindowController windowController;
    windowController.init();
    //windowController.toggleFullscreen();
    windowController.bindCameraControls(&camera);
    windowController.setWindowTitle("MineOx 0.2");
    GLFWwindow* window = windowController.getWindow();
    FontsLoader fontsLoader(windowController.getWidth(), windowController.getHeight());
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
    
    auto& fileHandler = FileHandler::getInstance();
    auto& pathProvider = PathProvider::getInstance();

    std::vector<std::filesystem::path> dirsToCheck = {
        pathProvider.getWorldChunksPath(world.getWorldName()),
        pathProvider.getDataPath(),
        pathProvider.getTextureFolderPath(),
        pathProvider.getBlocksTextureFolderPath()
    };

    for (const auto& dir : dirsToCheck) {
        if (!fileHandler.fileExists(dir)) {
            fileHandler.createDirectory(dir);
        }
    }

    Shader shader               = Shader::fromPaths(pathProvider.getMainShadersPath());
    Shader wireFrameCubeShader  = Shader::fromPaths(pathProvider.getWireFrameCubeShadersPath());
    Shader fontsShader          = Shader::fromPaths(pathProvider.getFontShadersPath());
    Shader depthShader          = Shader::fromPaths(pathProvider.getDepthShaderPath());

    TextureController::getInstance().initialize(PathProvider::getInstance().getBlocksTextureFolderPath());
    TextureController::getInstance().initializeTextures(shader.ID);
    
    fontsLoader.loadFont((PathProvider::getInstance().getFontsPath() / "arial.ttf").string());
    fontsLoader.loadCharacters(32);

    world.getShadowController().init();
    WireFrameCube wreFrameCube;
    wreFrameCube.init();
    
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
    glDepthFunc(GL_LESS);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_MULTISAMPLE);

    GLfloat largest_supported_anisotropy;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &largest_supported_anisotropy);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, largest_supported_anisotropy);

    glm::mat4 projection = glm::perspective(
        glm::radians(camera.FOV),
        (float)windowController.getWidth() / (float)windowController.getHeight(),
        0.1f,
        1000.0f
    );

    glm::mat4 model = glm::mat4(1.0f);

    while (!glfwWindowShouldClose(window)) {
        if(windowController.shouldToggleFullscreen) {
            windowController.toggleFullscreen();
        } else if(window) {
            float currentFrame = static_cast<float>(glfwGetTime());
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;
            
            world.getTimeOfDayController().update(deltaTime);
            auto skyLightInfo = world.getTimeOfDayController().getSkyLightInfo();

            glClearColor(skyLightInfo.skyColor.r, skyLightInfo.skyColor.g, skyLightInfo.skyColor.b, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            //stateController.handleInput(window);
            //stateController.update(deltaTime);

            world.getShadowController().update(skyLightInfo.lightDirection, camera.Position, world.getViewDistance());
            world.getShadowController().renderShadows(world.getChunkController().getLoadedChunks(), depthShader);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, windowController.getWidth(), windowController.getHeight());

            raycastHit = world.raycast(camera.Position, camera.Front, 5.0f);

            f3InfoScreen.fps = static_cast<int>(1.0f / deltaTime);
            f3InfoScreen.viewDistance = world.getViewDistance();
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

            shader.setMat4("lightSpaceMatrix", world.getShadowController().getSunLightSpaceMatrix());
            shader.setInt("shadowMap", 31);
            shader.setFloat("shadowMapSize", 4096.0f);
            shader.setVec3("lightColor", skyLightInfo.lightColor);
            shader.setVec3("lightDir", skyLightInfo.lightDirection);
            shader.setFloat("lightIntensity", skyLightInfo.lightIntensity);
            glActiveTexture(GL_TEXTURE31);
            glBindTexture(GL_TEXTURE_2D, world.getShadowController().getSunShadowMap());

            world.render(shader, skyLightInfo.lightDirection, skyLightInfo.lightColor);

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

                glBindVertexArray(wreFrameCube.VAO);
                glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);
            }

            //stateController.render();
            float wHeight = static_cast<float>(windowController.getHeight());
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            fontsLoader.RenderText(fontsShader, "FPS: " + f3InfoScreen.toString(f3InfoScreen.fps), 10.0f, wHeight - 30.0f, 1.0f, glm::vec3(0.0, 0.0f, 0.0f), fontsLoader.fontProjection);
            fontsLoader.RenderText(fontsShader, f3InfoScreen.toString(f3InfoScreen.blockPos, "Block Pos:"), 10.0f, wHeight - 60.0f, 1.0f, glm::vec3(0.0, 0.0f, 0.0f), fontsLoader.fontProjection);
            fontsLoader.RenderText(fontsShader, f3InfoScreen.toString(f3InfoScreen.chunkPos, "Chunk Pos:"), 10.0f, wHeight - 90.0f, 1.0f, glm::vec3(0.0, 0.0f, 0.0f), fontsLoader.fontProjection);
            fontsLoader.RenderText(fontsShader, f3InfoScreen.toString(f3InfoScreen.playerPos, "Coords: "), 10.0f, wHeight - 120.0f, 1.0f, glm::vec3(0.0, 0.0f, 0.0f), fontsLoader.fontProjection);
            fontsLoader.RenderText(fontsShader, "View distance: " + f3InfoScreen.toString(f3InfoScreen.viewDistance), 10.0f, wHeight - 150.0f, 1.0f, glm::vec3(0.0, 0.0f, 0.0f), fontsLoader.fontProjection);
            fontsLoader.RenderText(fontsShader, f3InfoScreen.facedBlockInfo, 10.0f, wHeight - 180.0f, 1.0f, glm::vec3(0.0, 0.0f, 0.0f), fontsLoader.fontProjection);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);

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