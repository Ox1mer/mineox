#pragma once
#include "AppState.h"

class GameState : public AppState {
public:
    void onEnter() override {
        // Подгрузка уровня
    }

    void update(float dt) override {
        // Игровая логика
    }

    void render() override {
        // Отрисовка мира
    }

    void handleInput(GLFWwindow* window) override {
        // Игровой ввод
    }

    void onExit() override {
        // Очистка
    }
};
