#include "Core/Application.hpp"
#include <chrono>
#include <iostream>

namespace Engine {
Application::Application(const ApplicationConfig& config) : m_Config(config), m_IsRunning(false) {

    std::cout << "[Engine] Initializing Application window: " << m_Config.windowTitle << "\n";

    // TODO: InitWindow(m_Config.windowWidth, m_Config.windowHeight, m_Config.windowTitle.c_str());
    // TODO: SetTargetFPS(m_Config.targetFPS);
}

Application::~Application() {
    std::cout << "[Engine] Shutting down Application...\n";
}

void Application::Run() {
    m_IsRunning = true;

    auto previousTime = std::chrono::high_resolution_clock::now();
    double lag = 0.0;
    const double MS_PER_UPDATE = 1.0 / m_Config.targetFPS;

    std::cout << "[Engine] Entering main game loop.\n";

    while (m_IsRunning) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsedTime = currentTime - previousTime;
        previousTime = currentTime;
        lag += elapsedTime.count();

        // TODO: if (WindowShouldClose()) { m_IsRunning = false; }

        PreUpdate(elapsedTime.count());

        while (lag >= MS_PER_UPDATE) {
            Update(MS_PER_UPDATE);
            lag -= MS_PER_UPDATE;
        }

        PostUpdate(elapsedTime.count());

        m_Registry.FlushQueues();
    }
}

void Application::PreUpdate(float dt) {
    // TODO: [Input] Poll Raylib for keyboard/mouse state
    // TODO: [Network] Poll UDP sockets for incoming server packets
}

void Application::Update(float dt) {
    for (auto const& pair : m_Registry.GetSystems()) {
        pair.second->Update(m_Registry, dt);
    }
}

void Application::PostUpdate(float dt) {
    // TODO: Call Raylib BeginDrawing()
    // TODO: Call Raylib ClearBackground()

    // TODO: Run ECS Render Systems (e.g., SpriteRendererSystem)
    // TODO: Call Raylib EndDrawing()
}
} // namespace Engine
