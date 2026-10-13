#include "Core/Application.hpp"
#include "Components/Name.hpp"
#include "Logging/Logger.hpp"
#include <chrono>
#include <raylib.h>
#include <string>

#include "Inputs/InputManager.hpp"

namespace Engine {
Application::Application(const ApplicationConfig& config) : m_Config(config), m_IsRunning(false) {
    log_info("[Engine] Initializing Application window: " + m_Config.windowTitle);

    unsigned int maximizedFlag = config.maximized ? FLAG_WINDOW_MAXIMIZED : 0;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | maximizedFlag);

    InitWindow(m_Config.windowWidth, m_Config.windowHeight, m_Config.windowTitle.c_str());
    SetTargetFPS(m_Config.targetFPS);
    SetWindowMinSize(320, 240);

    int virtualWidth = VirtualResolution::ResolveAxis(m_Config.virtualWidth, m_Config.windowWidth);
    int virtualHeight = VirtualResolution::ResolveAxis(m_Config.virtualHeight, m_Config.windowHeight);

    m_Registry.AddResource<VirtualResolution>(virtualWidth, virtualHeight);
    log_info("[Engine] Virtual resolution: " + std::to_string(virtualWidth) + "x" +
             std::to_string(virtualHeight));

    m_Registry.RegisterComponent<Name>();
}

Application::~Application() {
    log_info("[Engine] Shutting down Application...");
    // The canvas texture must be freed while the OpenGL context is still alive.
    if (m_Registry.HasResource<VirtualResolution>()) {
        m_Registry.GetResource<VirtualResolution>().Unload();
    }
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
    if (!m_Config.customRender && m_Registry.HasResource<VirtualResolution>()) {
        auto& virtualResolution = m_Registry.GetResource<VirtualResolution>();

        virtualResolution.UpdateLetterbox(GetScreenWidth(), GetScreenHeight());
        virtualResolution.ApplyMouseMapping();
    }

    if (m_Registry.HasResource<InputManager>()) {
        m_Registry.GetResource<InputManager>().Update();
    }
}

void Application::Update(float dt) {
    // Update order follows plugin registration (not the systems map); HierarchyPlugin before PositionPlugin.
    auto const& systems = m_Registry.GetSystems();

    for (std::type_index const& systemId : m_Registry.GetSystemUpdateOrder()) {
        systems.at(systemId)->Update(m_Registry, dt);
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
        auto& virtualResolution = m_Registry.GetResource<VirtualResolution>();
        auto const& systems = m_Registry.GetSystems();

        virtualResolution.Begin();
        ClearBackground(BLACK);
        for (std::type_index const& systemId : m_Registry.GetSystemRenderOrder()) {
            systems.at(systemId)->Render(m_Registry);
        }
        virtualResolution.End();

        BeginDrawing();
        ClearBackground(BLACK);
        virtualResolution.Present();

        // UI stays in window space, drawn on top of the letterboxed canvas.
        if (m_Config.onUI) {
            m_Config.onUI();
        }

        EndDrawing();
    }
}
} // namespace Engine
