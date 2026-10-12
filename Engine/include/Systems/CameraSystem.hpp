#ifndef ENGINE_SYSTEMS_CAMERASYSTEM_HPP_
#define ENGINE_SYSTEMS_CAMERASYSTEM_HPP_

#include "ECS/System.hpp"
#include <raylib.h>

namespace Engine {
class Registry;

/**
 * @class CameraSystem
 * @brief Moves cameras (follow, shake) and converts them into raylib cameras for rendering.
 */
class CameraSystem : public System {
  public:
    /**
     * @brief Moves every camera towards its follow target and decays its shake.
     * @param registry The ECS registry containing the cameras.
     * @param dt Delta time in seconds.
     */
    void Update(Registry& registry, float dt) override;

    /**
     * @brief Builds the raylib camera of the active camera for a given viewport.
     *
     * Returns an identity camera (world == screen) when there is no primary camera,
     * so games without a camera render exactly as before.
     * @param registry The ECS registry.
     * @param viewportWidth Width in pixels of the surface being drawn to.
     * @param viewportHeight Height in pixels of the surface being drawn to.
     */
    static Camera2D GetActiveCamera(Registry& registry, float viewportWidth,
                                    float viewportHeight);

    /**
     * @brief Converts a window position into a world position.
     * @param registry The ECS registry.
     * @param screen Position in window pixels.
     */
    static Vector2 ScreenToWorld(Registry& registry, Vector2 screen);

    /**
     * @brief Converts a world position into a window position.
     * @param registry The ECS registry.
     * @param world Position in world units.
     */
    static Vector2 WorldToScreen(Registry& registry, Vector2 world);
};
} // namespace Engine

#endif // ENGINE_SYSTEMS_CAMERASYSTEM_HPP_
