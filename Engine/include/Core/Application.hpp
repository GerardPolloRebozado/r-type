#ifndef ENGINE_CORE_APPLICATION_HPP_
#define ENGINE_CORE_APPLICATION_HPP_
#include "Core/VirtualResolution.hpp"
#include "ECS/Registry.hpp"
#include <functional>
#include <string>

namespace Engine {
/**
 * @brief Configuration passed to the engine when starting.
 * Contains window settings and target framerate.
 */
struct ApplicationConfig {
    int windowWidth = 1280;
    int windowHeight = 720;
    /**
     * @brief Virtual canvas width. 0 keeps the same width as the OS window.
     */
    int virtualWidth = 0;
    /**
     * @brief Virtual canvas height. 0 keeps the same height as the OS window.
     */
    int virtualHeight = 0;
    bool maximized = false;
    std::string windowTitle = "Game Engine";
    int targetFPS = 60;
    std::function<void()> onUI = nullptr;
    std::function<void(Registry&)> customRender = nullptr;
};

/**
 * @class Application
 * @brief The core framework of the engine. Owns the Registry and the Game Loop.
 */
class Application {
  public:
    Application(const ApplicationConfig& config);
    ~Application();

    /**
     * @brief Starts the main engine loop. Blocks until the application closes.
     */
    void Run();

    /**
     * @brief Gives Game Developers access to the ECS Registry to add components and systems.
     */
    Registry& GetRegistry() {
        return m_Registry;
    }

    /**
     * @brief Allows any system to shut down the engine.
     */
    void Quit() {
        m_IsRunning = false;
    }

    /**
     * @brief Render texture of the virtual game canvas.
     * @return The canvas texture owned by the VirtualResolution resource.
     * @throws ErrorEcs if that resource is missing.
     */
    RenderTexture2D& GetGameTexture() {
        return m_Registry.GetResource<VirtualResolution>().GetTexture();
    }

  private:
    ApplicationConfig m_Config;
    Registry m_Registry;
    bool m_IsRunning;

    void PreUpdate(float dt);
    void Update(float dt);
    void PostUpdate(float dt);
};
} // namespace Engine

#endif // ENGINE_CORE_APPLICATION_HPP_
