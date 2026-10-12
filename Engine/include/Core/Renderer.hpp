#ifndef ENGINE_CORE_RENDERER_HPP_
#define ENGINE_CORE_RENDERER_HPP_

#include "ECS/Registry.hpp"
#include <functional>

namespace Engine {
/**
 * @brief Draws every system in two passes onto the current render target.
 *
 * 1. World pass: each system's Render(), inside the active camera.
 * 2. Screen pass: each system's RenderScreen(), in plain pixels (HUD).
 *
 * Systems are drawn in registration order ( @ref Registry::GetSystemRenderOrder).
 * Shared by the game loop and the Editor scene view so both draw the same thing.
 * Does not call BeginDrawing/EndDrawing nor clear the background.
 * @param registry The ECS registry.
 * @param viewportWidth Width in pixels of the surface being drawn to.
 * @param viewportHeight Height in pixels of the surface being drawn to.
 * @param worldOverlay Optional extra world-space drawing done after the systems,
 *        still inside the camera.
 */
void RenderScene(Registry& registry, float viewportWidth, float viewportHeight,
                 const std::function<void()>& worldOverlay = nullptr);
} // namespace Engine

#endif // ENGINE_CORE_RENDERER_HPP_
