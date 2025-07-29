#pragma once

#include "IInputController.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include "GLFW/glfw3.h"
#include "ServiceLocator.h"

#include "ChatController.h"

class PCInputController : public IInputController
{
private:
    inline static float leftMouseCooldown = 0.0f;
    inline static float rightMouseCooldown = 0.0f;
    const float cooldownTime = 0.2f;

    bool leftMousePressedLastFrame = false;
    bool rightMousePressedLastFrame = false;
    bool escapePressed = false;
    bool backspacePressed = false;

public:

    bool shouldTakeScreenshot = false;

    void processKeyInput(Camera* camera, float deltaTime, std::optional<RaycastHit>& raycastHit, Blocks& choosedBlock, GLFWwindow* window, ChatController& chatController) override {
        if (chatController.getVisibility()) {
            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                if (!escapePressed) {
                    chatController.setvisible(false);
                    escapePressed = true;
                }
            } else {
                escapePressed = false;
            }

            if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS) {
                if (!backspacePressed) {
                    if (!chatController.getInputBuffer().empty()) {
                        chatController.removeLastSymbol();
                    }
                    backspacePressed = true;
                }
            } else {
                backspacePressed = false;
            }
            
            if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS) {
                shouldTakeScreenshot = true;
            }

            if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
                if (!chatController.getInputBuffer().empty()) {
                    chatController.addMessage(chatController.getInputBuffer());
                    chatController.clearInputBuffer();
                }
            }

        } else {
            if (leftMouseCooldown > 0.0f)
                leftMouseCooldown -= deltaTime;
            if (rightMouseCooldown > 0.0f)
                rightMouseCooldown -= deltaTime;

            if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
                chatController.setvisible(true);

            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                camera->ProcessKeyboard(FORWARD, deltaTime);
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                camera->ProcessKeyboard(BACKWARD, deltaTime);
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                camera->ProcessKeyboard(LEFT, deltaTime);
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                camera->ProcessKeyboard(RIGHT, deltaTime);
            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
                camera->ProcessKeyboard(UP, deltaTime);
            if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
                camera->ProcessKeyboard(DOWN, deltaTime);

            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                if (!escapePressed) {
                    glfwSetWindowShouldClose(window, true);
                    escapePressed = true;
                }
            } else {
                escapePressed = false;
            }

            if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS) {
                shouldTakeScreenshot = true;
            }

            bool leftPressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
            if (leftPressed && raycastHit) {
                if (!leftMousePressedLastFrame) {
                    ServiceLocator::GetWorld()->getChunkController().breakBlock(BlockPos(raycastHit.value().blockPos));
                    leftMouseCooldown = cooldownTime;
                } else if (leftMouseCooldown <= 0.0f) {
                    ServiceLocator::GetWorld()->getChunkController().breakBlock(BlockPos(raycastHit.value().blockPos));
                    leftMouseCooldown = cooldownTime;
                }
            }
            leftMousePressedLastFrame = leftPressed;

            bool rightPressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
            if (rightPressed && raycastHit) {
                if (!rightMousePressedLastFrame) {
                    BlockPos newBlockPos(raycastHit.value().blockPos + raycastHit.value().faceNormal);
                    ServiceLocator::GetWorld()->getChunkController().setBlock(newBlockPos, choosedBlock);
                    rightMouseCooldown = cooldownTime;
                } else if (rightMouseCooldown <= 0.0f) {
                    BlockPos newBlockPos(raycastHit.value().blockPos + raycastHit.value().faceNormal);
                    ServiceLocator::GetWorld()->getChunkController().setBlock(newBlockPos, choosedBlock);
                    rightMouseCooldown = cooldownTime;
                }
            }
            rightMousePressedLastFrame = rightPressed;
        }
    }

    bool getShouldTakeScreenshot() const override {
        return shouldTakeScreenshot;
    }

    void setShouldTakeScreenshot(bool value) override {
        shouldTakeScreenshot = value;
    }

};