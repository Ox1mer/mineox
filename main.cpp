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
#include "ScreenshotCreator.h"

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
#include "GLSettingsController.h"

#ifdef _WIN32
    #include "PCInputController.h"
#endif

std::unique_ptr<IInputController> inputController =
#ifdef _WIN32
    std::make_unique<PCInputController>();
#else
    nullptr;
#endif

#include "CommandsIncluder.h"

WindowContext ctx;

void setupSceneShader(Shader& shader, 
                      const Camera& camera, 
                      const glm::mat4& projection,
                      const glm::mat4& model,
                      World& world,
                      const SkyLightInfo& skyLightInfo);

float deltaTime = 0.0f;
float lastFrame = 0.0f;
std::optional<RaycastHit> raycastHit;

int shadowUpdateFrameCounter = 0;
const int shadowUpdateInterval = 10;

// Later create an entity class - and then Player will be an entity and will have a camera and choosed block controls
Camera camera;

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
    ScreenshotCreator screenshotCreator;

    auto& fileHandler = FileHandler::getInstance();
    auto& pathProvider = PathProvider::getInstance();
    auto& openGLSettingsController = GLSettingsController::getInstance();

    if (!window) {
        Logger::getInstance().Log("Failed to create GLFW window", LogLevel::Critical, LogOutput::Both);
        return -1;
    }

    openGLSettingsController.initializeOpenGLSettings(); // Initialize OpenGL settings

    StateController stateController;
    stateController.changeState(std::make_unique<MainMenuState>());

    camera = Camera();
    World world(12345, "New world"); // Later, get seed and world name from user input
    ServiceLocator::ProvideWorld(&world);
    //world.initWorld(camera.Position);

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
    Shader chatShader           = Shader::fromPaths(pathProvider.getChatShadersPath());

    TextureController::getInstance().initialize(PathProvider::getInstance().getBlocksTextureFolderPath(), {
        Blocks::Dirt, Blocks::Gneiss,
        Blocks::Gravel, Blocks::Migmatite, Blocks::Sand,
        Blocks::SporeMoss, Blocks::Stone
    });
    TextureController::getInstance().initializeTextures(shader.ID, {
        Blocks::Dirt, Blocks::Gneiss,
        Blocks::Gravel, Blocks::Migmatite, Blocks::Sand,
        Blocks::SporeMoss, Blocks::Stone
    });
    
    float symbolSize = 32.0f; // Size of the font symbols

    fontsLoader.loadFont((PathProvider::getInstance().getFontsPath() / "arial.ttf").string());
    fontsLoader.loadCharacters(symbolSize);

    world.getShadowController().init();
    WireFrameCube wireFrameCube;
    wireFrameCube.init();

    ChatController chatController = ChatController(chatShader);
    chatController.init();
    chatController.setvisible(true);

    ServiceLocator::ProvideChatController(&chatController);
    ServiceLocator::ProvideCamera(&camera);

    glfwSetWindowUserPointer(window, &chatController);
    windowController.registerCharCallback();

    ctx.camera = &camera;
    ctx.chat = &chatController;

    glfwSetWindowUserPointer(window, &ctx);
    
    glm::mat4 projection = glm::perspective(
        glm::radians(camera.FOV),
        (float)windowController.getWidth() / (float)windowController.getHeight(),
        0.1f,
        1000.0f
    );

    glm::mat4 model = glm::mat4(1.0f);

    while (!glfwWindowShouldClose(window)) {
        if(window) {
            float currentFrame = static_cast<float>(glfwGetTime());
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            shadowUpdateFrameCounter++;
            
            world.getTimeOfDayController().update(deltaTime);
            auto skyLightInfo = world.getTimeOfDayController().getSkyLightInfo();
            
            glClearColor(skyLightInfo.skyColor.r, skyLightInfo.skyColor.g, skyLightInfo.skyColor.b, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            //stateController.handleInput(window);
            //stateController.update(deltaTime);

            if (shadowUpdateFrameCounter >= shadowUpdateInterval) {
                shadowUpdateFrameCounter = 0;

                world.getShadowController().update(
                    skyLightInfo.lightDirection,
                    camera.Position,
                    world.getViewDistance()
                );
            }

            world.getShadowController().renderShadows(world.getChunkController().getLoadedChunks(), depthShader);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            // It will change viewport size, so we need to set it again
            glViewport(0, 0, windowController.getWidth(), windowController.getHeight());

            raycastHit = world.raycast(camera.Position, camera.Front, 5.0f);

            f3InfoScreen.update(deltaTime, camera, world, raycastHit);

            inputController->processKeyInput(&camera, deltaTime, raycastHit, camera.choosedBlock, window, chatController);
            
            world.update(PlayerPos(glm::ivec3(camera.Position.x, camera.Position.y, camera.Position.z)));

            setupSceneShader(shader, camera, projection, model, world, skyLightInfo);
            world.render(shader, skyLightInfo.lightDirection, skyLightInfo.lightColor);

            if (raycastHit) {
                wireFrameCube.render(glm::vec3(raycastHit->blockPos), camera, projection, wireFrameCubeShader);
            }
            
            //stateController.render();
            
            openGLSettingsController.disableCullDepth();
            f3InfoScreen.render(fontsLoader, fontsShader, static_cast<float>(windowController.getHeight()));
            openGLSettingsController.enableCullDepth();
            
            openGLSettingsController.disableCullDepth();
            chatController.render(windowController.getWidth(), windowController.getHeight(), symbolSize, fontsLoader, fontsShader);
            openGLSettingsController.enableCullDepth();

            if(inputController->getShouldTakeScreenshot()) {
                screenshotCreator.doTheScreenshotAndSave(window);
                Logger::getInstance().Log("Screenshot taken", LogLevel::Info, LogOutput::Both, LogWriteMode::Append);
                inputController->setShouldTakeScreenshot(false);
            }

            if (chatController.getVisibility()) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            } else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
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

void setupSceneShader(Shader& shader, 
                      const Camera& camera, 
                      const glm::mat4& projection,
                      const glm::mat4& model,
                      World& world,
                      const SkyLightInfo& skyLightInfo) {
    shader.use();
    shader.setMat4("view", camera.GetViewMatrix());
    shader.setMat4("projection", projection);
    shader.setMat4("model", model);

    shader.setMat4("lightSpaceMatrix", world.getShadowController().getSunLightSpaceMatrix());
    shader.setInt("shadowMap", 31);
    shader.setFloat("shadowMapSize", 8192.0f);
    shader.setVec3("lightColor", skyLightInfo.lightColor);
    shader.setVec3("lightDir", skyLightInfo.lightDirection);
    shader.setFloat("lightIntensity", skyLightInfo.lightIntensity);

    glActiveTexture(GL_TEXTURE31);
    glBindTexture(GL_TEXTURE_2D, world.getShadowController().getSunShadowMap());
}