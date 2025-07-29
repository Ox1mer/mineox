#pragma once

// This interface defines the methods for an input controller
// It should be implemented by any class that handles input for the application

#include "Camera.h"
#include "RayCastHit.h"
#include "Blocks.h"
#include <optional>
#include <GLFW/glfw3.h>

#include "ChatController.h"

class IInputController
{
public:
    virtual ~IInputController() = default;

    virtual bool getShouldTakeScreenshot() const = 0;
    virtual void setShouldTakeScreenshot(bool value) = 0;

    virtual void processKeyInput(Camera* camera, float deltaTime, std::optional<RaycastHit>& raycastHit, Blocks& choosedBlock,  GLFWwindow* window, ChatController& chatController) = 0;
};