#pragma once

#include "Camera.h"
#include "ChatController.h"

struct WindowContext {
    Camera* camera = nullptr;
    ChatController* chat = nullptr;
};