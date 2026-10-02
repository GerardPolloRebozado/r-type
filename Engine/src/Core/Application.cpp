#include "Core/Application.hpp"
#include <chrono>
#include <iostream>
#include <raylib.h>

namespace Engine {
Application::Application(const ApplicationConfig& config) : m_Config(config), m_IsRunning(false) {

    std::cout << "[Engine] Initializing Application window: " << m_Config.windowTitle << "\n";
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(m_Config.windowWidth, m_Config.windowHeight, m_Config.windowTitle.c_str());

    SetTargetFPS(m_Config.targetFPS);
}

Application::~Application() {
    std::cout << "[Engine] Shutting down Application...\n";
    CloseWindow();
}

void Application::Run() {
    m_IsRunning = true;

    auto previousTime = std::chrono::high_resolution_clock::now();
    double lag = 0.0;
    const double MS_PER_UPDATE = 1.0 / m_Config.targetFPS;

    std::cout << "[Engine] Entering main game loop.\n";

    while (m_IsRunning && !WindowShouldClose()) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsedTime = currentTime - previousTime;
        previousTime = currentTime;
        lag += elapsedTime.count();

        PreUpdate(elapsedTime.count());

        while (lag >= MS_PER_UPDATE) {
            Update(MS_PER_UPDATE);
            lag -= MS_PER_UPDATE;
        }

        PostUpdate(elapsedTime.count());

        m_Registry.FlushQueues();
    }
}

void Application::PreUpdate(float dt) {}

void Application::Update(float dt) {
    for (auto const& pair : m_Registry.GetSystems()) {
        pair.second->Update(m_Registry, dt);
    }
}

void Application::PostUpdate(float dt) {
    BeginDrawing();
    ClearBackground(DARKGRAY);

    // Run ECS Render Systems
    for (auto const& pair : m_Registry.GetSystems()) {
        pair.second->Render(m_Registry);
    }

    // Invoke UI Hook (Editor uses this for ImGui)
    if (m_Config.onUI) {
        m_Config.onUI();
    }

    EndDrawing();
}
} // namespace Engine
