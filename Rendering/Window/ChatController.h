#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"
#include "FontsLoader.h"
#include "GLSettingsController.h"

#include "ChatCommands.h"
#include "ChatCommandFactory.h"

class ChatController {
private:
    std::vector<std::string> messages;
    size_t maxMessages = 20;
    bool isVisible = false;

    Shader shader;

    GLuint quadVAO = 0;
    GLuint quadVBO = 0;

    float offsetX = 0.0f;
    float offsetY = 0.0f;

    static constexpr float inputBoxHeight = 48.0f; // Height of the input

    GLSettingsController& openGLSettingsController = GLSettingsController::getInstance();

    std::string inputBuffer;

    std::vector<float> getMessagesHistoryBoxVertices(float x, float y, float width, float height) {
        return {
            x, y,
            x + width, y,
            x + width, y + height + inputBoxHeight,

            x, y,
            x + width, y + height + inputBoxHeight,
            x, y + height + inputBoxHeight
        };
    };

    std::vector<float> getInputBoxVertices(float x, float y, float width, float height) {
        return {
            x + 0, y + inputBoxHeight,                // левый верхний
            x + width, y + inputBoxHeight,            // правый верхний
            x + width, y + 0,                         // правый нижний

            x + 0, y + inputBoxHeight,                // левый верхний
            x + width, y + 0,                         // правый нижний
            x + 0, y + 0                              // левый нижний
        };
    }

    std::vector<std::string> wrapMessage(const std::string& message, float maxWidth, float scale, FontsLoader& fontsLoader) {
        std::vector<std::string> lines;
        std::string currentLine;
        float lineWidth = 0.0f;

        auto it = message.begin(), end = message.end();
        std::string wordBuffer;
        float wordWidth = 0.0f;

        while (it != end) {
            auto prevIt = it;
            char32_t cp = utf8::next(it, end);
            Character& ch = fontsLoader.getCharacters()[cp];
            float w = (ch.Advance >> 6) * scale;

            if (cp == U' ') {
                if (lineWidth + wordWidth > maxWidth) {
                    if (!currentLine.empty()) lines.push_back(currentLine);
                    currentLine = wordBuffer + " ";
                    lineWidth = wordWidth + w;
                } else {
                    currentLine += wordBuffer + " ";
                    lineWidth += wordWidth + w;
                }
                wordBuffer.clear();
                wordWidth = 0.0f;
            } else {
                wordBuffer.append(prevIt, it);
                wordWidth += w;

                if (wordWidth > maxWidth) {
                    if (!currentLine.empty()) lines.push_back(currentLine);
                    lines.push_back(wordBuffer);
                    currentLine.clear();
                    wordBuffer.clear();
                    wordWidth = 0.0f;
                    lineWidth = 0.0f;
                }
            }
        }

        if (!wordBuffer.empty()) {
            if (lineWidth + wordWidth > maxWidth) {
                if (!currentLine.empty()) lines.push_back(currentLine);
                currentLine = wordBuffer;
            } else {
                currentLine += wordBuffer;
            }
        }

        if (!currentLine.empty()) lines.push_back(currentLine);

        return lines;
    }


public:
    void setOffset(float x, float y) {
        offsetX = x;
        offsetY = y;
    }

    ChatController(Shader& shader, float offsetX = 0.0f, float offsetY = 0.0f)
        : shader(shader)
    {
        setOffset(offsetX, offsetY);
    }

    ~ChatController() {
        glDeleteVertexArrays(1, &quadVAO);
        glDeleteBuffers(1, &quadVBO);
    }

    void init() {
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);

        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, nullptr, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

        glBindVertexArray(0);
    }

    bool getVisibility() {
        return isVisible;
    }

    void setvisible(bool visible) {
        isVisible = visible;
    }

    void render(float screenWidth, float screenHeight, float symbolSize, FontsLoader& fontsLoader, Shader& fontsShader) {
        if (!isVisible) return;
        if (symbolSize > inputBoxHeight) {
            symbolSize = inputBoxHeight; // Ensure symbol size does not exceed input box height
        }

        glm::mat4 projection = glm::ortho(0.0f, screenWidth, 0.0f, screenHeight);

        shader.use();
        shader.setMat4("projection", projection);

        float width = screenWidth * 0.5f;
        float height = screenHeight * 0.5f;

        float x = offsetX;
        float y = offsetY;

        std::vector<float> vertices = getMessagesHistoryBoxVertices(x, y, width, height);
        std::vector<float> bottomRect = getInputBoxVertices(x, y, width, height);

        std::vector<float> bothRects;

        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        bothRects.reserve(vertices.size() + bottomRect.size());
        bothRects.insert(bothRects.end(), vertices.begin(), vertices.end());
        bothRects.insert(bothRects.end(), bottomRect.begin(), bottomRect.end());

        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, bothRects.size() * sizeof(float), bothRects.data());

        shader.setVec4("overlayColor", glm::vec4(0.0f, 0.0f, 0.0f, 0.6f));
        glDrawArrays(GL_TRIANGLES, 0, 6);

        shader.setVec4("overlayColor", glm::vec4(0.0f, 0.0f, 0.0f, 0.8f));
        glDrawArrays(GL_TRIANGLES, 6, 6);

        // Render messages
        shader.setVec4("overlayColor", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        if (inputBuffer.empty()) {
            openGLSettingsController.disableCullDepth();
            fontsLoader.RenderText(
                fontsShader,
                "Type your message here...",
                x + 10.0f, y + inputBoxHeight / 2.0f - 0.2f * inputBoxHeight,
                0.7f,
                glm::vec3(192.0f / 255.0f, 192.0f / 255.0f, 192.0f / 255.0f),
                fontsLoader.fontProjection
            );
            openGLSettingsController.enableCullDepth();
        } else {
            openGLSettingsController.disableCullDepth();
            fontsLoader.RenderText(
                fontsShader,
                inputBuffer,
                x + 10.0f, y + inputBoxHeight / 2.0f - 0.2f * inputBoxHeight,
                0.7f,
                glm::vec3(0.980f, 0.980f, 0.980f),
                fontsLoader.fontProjection
            );
            openGLSettingsController.enableCullDepth();
        }

        float lineSpacing = 1.2f;
        float lineHeight = symbolSize * lineSpacing;
        float textAreaHeight = height - inputBoxHeight;
        size_t maxLines = static_cast<size_t>(std::floor(textAreaHeight / lineHeight));

        std::vector<std::string> lines;

        for (int i = messages.size() - 1; i >= 0; --i) {
            auto wrapped = wrapMessage(messages[i], width - 20.0f, 0.6f, fontsLoader);
            lines.insert(lines.begin(), wrapped.begin(), wrapped.end());
            if (lines.size() >= maxLines) break;
        }

        if (lines.size() > maxLines) {
            lines.erase(lines.begin(), lines.begin() + (lines.size() - maxLines));
        }

        for (size_t i = 0; i < lines.size(); ++i) {
            float yOffset = y + inputBoxHeight + ((lines.size() - i - 1) * symbolSize) + symbolSize / 2.0f;

            openGLSettingsController.disableCullDepth();
            fontsLoader.RenderText(
                fontsShader,
                lines[i],
                x + 10.0f, yOffset,
                0.6f,
                glm::vec3(0.949f, 0.953f, 0.957f),
                fontsLoader.fontProjection
            );
            openGLSettingsController.enableCullDepth();
        }
        glBindVertexArray(0);
    }

    void addSymbol(const std::string& symbol) {
        inputBuffer += symbol;
    }

    void removeLastSymbol() {
        if (!inputBuffer.empty()) {
            removeLastUtf8Char(inputBuffer);
        }
    }

    void clearInputBuffer() {
        inputBuffer.clear();
    }

    std::string getInputBuffer() const {
        return inputBuffer;
    }

    void addMessage(std::string message) {
        if (messages.size() >= maxMessages) {
            messages.erase(messages.begin());
        }
        messages.push_back(message);
    }

    std::string utf32ToUtf8(char32_t codepoint) {
            std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
            return conv.to_bytes(codepoint);
        }
    
    void enterPressed() {
        if (inputBuffer.empty()) return;

        auto message = inputBuffer;
        clearInputBuffer();

        if (message[0] == '/') {
            handleCommand(message.substr(1));
        } else {
            addMessage(message);
        }
    }

    void clearMessages() {
        messages.clear();
    }

    void handleCommand(const std::string& commandLine) {
        std::istringstream iss(commandLine);
        std::string commandName;
        iss >> commandName;

        auto it = ChatCommandNameMap.find(commandName);
        if (it == ChatCommandNameMap.end()) {
            addMessage("Unknown command: " + commandName);
            return;
        }

        ChatCommandID id = it->second;

        auto command = ChatCommandFactory::getInstance().create(id);
        if (command) {
            std::string args;
            std::getline(iss, args);
            command->execute(args);
        } else {
            addMessage("Command not implemented: " + commandName);
        }
    }

private:
    void removeLastUtf8Char(std::string& str) {
        if (str.empty()) return;

        int i = (int)str.size() - 1;
        while (i >= 0 && ((str[i] & 0xC0) == 0x80)) {
            i--;
        }
        if (i >= 0) {
            str.erase(i);
        }
    }
};