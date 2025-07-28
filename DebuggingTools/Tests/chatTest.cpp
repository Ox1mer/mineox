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

#include "ChatController.h"

float deltaTime = 0.0f;
float lastFrame = 0.0f;
std::optional<RaycastHit> raycastHit;

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

    Shader shader               = Shader::fromPaths(pathProvider.getMainShadersPath());
    Shader wireFrameCubeShader  = Shader::fromPaths(pathProvider.getWireFrameCubeShadersPath());
    Shader fontsShader          = Shader::fromPaths(pathProvider.getFontShadersPath());
    Shader depthShader          = Shader::fromPaths(pathProvider.getDepthShaderPath());
    Shader chatShader           = Shader::fromPaths(pathProvider.getChatShadersPath());

    TextureController::getInstance().initialize(PathProvider::getInstance().getBlocksTextureFolderPath());
    TextureController::getInstance().initializeTextures(shader.ID);
    
    fontsLoader.loadFont((PathProvider::getInstance().getFontsPath() / "arial.ttf").string());
    fontsLoader.loadCharacters(32);

    ChatController chatController = ChatController(chatShader);
    chatController.setvisible(true);
    
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glClearColor(0.4f, 0.7f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        openGLSettingsController.disableCullDepth();
        chatController.render(windowController.getWidth(), windowController.getHeight());
        openGLSettingsController.enableCullDepth();

        glfwPollEvents();
        GLResourceDeleter::getInstance().processDeletes();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    Logger::getInstance().Log("Application shutdown", LogLevel::Warning, LogOutput::Both, LogWriteMode::Append);
    windowController.shutdown();
    return 0;
}   