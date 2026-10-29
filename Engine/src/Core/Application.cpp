#include "Core/Application.hpp"
#include "Components/Name.hpp"
#include "Components/PrefabInstance.hpp"
#include "Core/LaunchOptions.hpp"
#include "Core/Renderer.hpp"
#include "Logging/Logger.hpp"
#include "Resources/SceneManager.hpp"
#include <chrono>
#include <raylib.h>
#include <string>

#include "Inputs/InputManager.hpp"

namespace Engine {
Application::Application(const ApplicationConfig& config) : m_Config(config), m_IsRunning(false) {
    if (m_Config.headless) {
        log_info("[Engine] Starting in headless mode (no window, no rendering).");
    } else {
        log_info("[Engine] Initializing Application window: " + m_Config.windowTitle);

        unsigned int maximizedFlag = config.maximized ? FLAG_WINDOW_MAXIMIZED : 0;

        SetConfigFlags(FLAG_WINDOW_RESIZABLE | maximizedFlag);

        InitWindow(m_Config.windowWidth, m_Config.windowHeight, m_Config.windowTitle.c_str());
        SetTargetFPS(m_Config.targetFPS);
        SetWindowMinSize(320, 240);
    }

    int virtualWidth = VirtualResolution::ResolveAxis(m_Config.virtualWidth, m_Config.windowWidth);
    int virtualHeight =
        VirtualResolution::ResolveAxis(m_Config.virtualHeight, m_Config.windowHeight);

    // no window on headless
    if (!m_Config.headless) {
        m_Registry.AddResource<VirtualResolution>(virtualWidth, virtualHeight);
        log_info("[Engine] Virtual resolution: " + std::to_string(virtualWidth) + "x" +
                 std::to_string(virtualHeight));
    }

    m_Registry.AddResource<LaunchOptions>(LaunchOptions{config.args});

    m_Registry.RegisterComponent<Name>();
    m_Registry.RegisterComponent<PrefabInstance>();
}

Application::~Application() {
    log_info("[Engine] Shutting down Application...");
    // The canvas texture must be freed while the OpenGL context is still alive.
    if (m_Registry.HasResource<VirtualResolution>()) {
        m_Registry.GetResource<VirtualResolution>().Unload();
    }
    if (!m_Config.headless) {
        CloseWindow();
    }
}

void Application::Run() {
    m_IsRunning = true;

    auto previousTime = std::chrono::high_resolution_clock::now();
    double lag = 0.0;
    const double MS_PER_UPDATE = 1.0 / m_Config.targetFPS;

    log_info("[Engine] Entering main game loop.");

    // A scene requested before Run() is built now, so the first frame
    // already shows it.
    ApplyPendingScene();

    while (m_IsRunning && (m_Config.headless || !WindowShouldClose())) {
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
        // After FlushQueues: switch scenes only once the frame is fully done.
        ApplyPendingScene();
    }
}

void Application::ApplyPendingScene() {
    if (m_Registry.HasResource<SceneManager>()) {
        m_Registry.GetResource<SceneManager>().ApplyPending(m_Registry);
    }
}

void Application::PreUpdate(float dt) {
    if (!m_Config.headless && !m_Config.customRender &&
        m_Registry.HasResource<VirtualResolution>()) {
        auto& virtualResolution = m_Registry.GetResource<VirtualResolution>();

        virtualResolution.UpdateLetterbox(GetScreenWidth(), GetScreenHeight());
        virtualResolution.ApplyMouseMapping();
    }

    if (m_Registry.HasResource<InputManager>()) {
        m_Registry.GetResource<InputManager>().Update();
    }
}

void Application::Update(float dt) {
    // Update order follows plugin registration (not the systems map); HierarchyPlugin before
    // PositionPlugin.
    auto const& systems = m_Registry.GetSystems();

    for (std::type_index const& systemId : m_Registry.GetSystemUpdateOrder()) {
        systems.at(systemId)->Update(m_Registry, dt);
    }
}

void Application::PostUpdate(float dt) {
    if (m_Config.headless) {
        return; // No window, no canvas: systems already ran in Update().
    }
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

        virtualResolution.Begin();
        ClearBackground(BLACK);
        // Camera and HUD draw into the canvas. The window only receives the letterbox.
        RenderScene(m_Registry, static_cast<float>(virtualResolution.GetWidth()),
                    static_cast<float>(virtualResolution.GetHeight()));
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
