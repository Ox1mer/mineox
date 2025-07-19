#pragma once
#include "AppState.h"

class MainMenuState : public AppState {
public:
    void onEnter() override {
        // Музыка, фон, GUI и т.д.
    }

    void update(float dt) override {
        // Проверка кнопок, переход в GameState
    }

    void render() override {
        // Рисуем меню
    }

    void handleInput(GLFWwindow* window) override {
        // Нажатие ESC или Enter
    }

    void onExit() override {
        // Очистка GUI и т.п.
    }
};
