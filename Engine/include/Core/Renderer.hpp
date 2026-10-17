#ifndef ENGINE_CORE_RENDERER_HPP_
#define ENGINE_CORE_RENDERER_HPP_

#include "ECS/Registry.hpp"
#include <functional>
#include <optional>
#include <raylib-cpp.hpp>

namespace Engine {
/**
 * @brief Draws every system in two passes onto the current render target.
 *
 * 1. World pass, inside the active camera.
 * 2. Screen pass, in plain pixels (HUD): entities tagged with @ref ScreenSpace.
 *
 * In each pass, the entities of every @ref LayeredRenderSystem (sprites, shapes, text) are
 * drawn first, sorted by @ref Position::z (higher z on top), then by system registration
 * order ( @ref Registry::GetSystemRenderOrder), then by entity id. After that, each system's
 * Render() / RenderScreen() runs in registration order, for systems that draw on their own.
 * Shared by the game loop and the Editor scene view so both draw the same thing.
 * Does not call BeginDrawing/EndDrawing nor clear the background.
 * @param registry The ECS registry.
 * @param viewportWidth Width in pixels of the surface being drawn to.
 * @param viewportHeight Height in pixels of the surface being drawn to.
 * @param worldOverlay Optional extra world-space drawing done after the systems,
 *        still inside the camera.
 * @param cameraOverride Optional camera used instead of the active one for the world pass.
 * @param screenCameraOverride Optional camera applied to the screen-space pass, so callers
 *        drawing to a surface larger than the canvas (the Editor scene view) can preview the
 *        HUD where the game would show it. Omitted => HUD drawn in plain pixels, as at runtime.
 */
void RenderScene(Registry& registry, float viewportWidth, float viewportHeight,
                 const std::function<void()>& worldOverlay = nullptr,
                 std::optional<Camera2D> cameraOverride = std::nullopt,
                 std::optional<Camera2D> screenCameraOverride = std::nullopt);
} // namespace Engine

#endif // ENGINE_CORE_RENDERER_HPP_
