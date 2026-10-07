#include "Core/Application.hpp"
#include "Logging/Logger.hpp"
#include <chrono>
#include <raylib.h>

#include "Inputs/InputManager.hpp"

namespace Engine {
Application::Application(const ApplicationConfig& config) : m_Config(config), m_IsRunning(false) {
    log_info("[Engine] Initializing Application window: " + m_Config.windowTitle);

    unsigned int maximizedFlag = config.maximized ? FLAG_WINDOW_MAXIMIZED : 0;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | maximizedFlag);

    InitWindow(m_Config.windowWidth, m_Config.windowHeight, m_Config.windowTitle.c_str());
    SetTargetFPS(m_Config.targetFPS);
}

Application::~Application() {
    log_info("[Engine] Shutting down Application...");
    CloseWindow();
}

void Application::Run() {
    m_IsRunning = true;

    auto previousTime = std::chrono::high_resolution_clock::now();
    double lag = 0.0;
    const double MS_PER_UPDATE = 1.0 / m_Config.targetFPS;

    log_info("[Engine] Entering main game loop.");

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

void Application::PreUpdate(float dt) {
    if (m_Registry.HasResource<InputManager>()) {
        m_Registry.GetResource<InputManager>().Update();
    }
}

void Application::Update(float dt) {
    for (auto const& pair : m_Registry.GetSystems()) {
        pair.second->Update(m_Registry, dt);
    }
}

void Application::PostUpdate(float dt) {
    if (m_Config.customRender) {
        m_Config.customRender(m_Registry);

        BeginDrawing();
        ClearBackground(BLACK);
        if (m_Config.onUI) {
            m_Config.onUI();
        }
        EndDrawing();
    } else {
        BeginDrawing();
        ClearBackground(BLACK);

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
}
} // namespace Engine
