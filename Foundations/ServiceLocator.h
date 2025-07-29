#pragma once

#include <memory>
#include "World.h"
#include "ChatController.h"
#include "Camera.h"

class World;

class ServiceLocator {
private:
    inline static World* worldInstance = nullptr;
    inline static ChatController* chatControllerInstance = nullptr;
    inline static Camera* cameraInstance = nullptr;
public:
    ServiceLocator() = delete;

    static void ProvideWorld(World* world) {
        worldInstance = world;
    }

    static World* GetWorld() {
        return worldInstance;
    }

    static void ProvideChatController(ChatController* chatController) {
        chatControllerInstance = chatController;
    }

    static ChatController* GetChatController() {
        return chatControllerInstance;
    }

    static void ProvideCamera(Camera* camera) {
        cameraInstance = camera;
    }

    static Camera* getCamera() {
        return cameraInstance;
    }
};
