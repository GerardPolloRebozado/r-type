#ifndef ENGINE_COMPONENTS_CAMERACOMPONENT_HPP_
#define ENGINE_COMPONENTS_CAMERACOMPONENT_HPP_

#include "ECS/Entity.hpp"
#include <optional>
#include <raylib.h>

namespace Engine {
/**
 * @struct CameraComponent
 * @brief 2D camera that decides which part of the world is shown on screen.
 *
 * The camera reuses the transform of its entity:
 * - @ref Position is the world point drawn at the center of the viewport.
 * - @ref Rotation is the rotation of the view, in degrees.
 *
 * Only one camera is used for rendering: the @ref primary camera with the lowest entity id.
 * Entities tagged with @ref ScreenSpace ignore the camera (HUD, menus...).
 */
struct CameraComponent {
    /** @brief Zoom factor (1 = no zoom, 2 = everything twice as big). */
    float zoom;

    /** @brief When true, this camera is a candidate for rendering. */
    bool primary;

    /** @brief Entity whose @ref Position the camera follows. No follow when empty. */
    std::optional<Entity> followTarget;

    /**
     * @brief How fast the camera catches up with @ref followTarget, per second.
     * 0 snaps instantly to the target.
     */
    float followSpeed;

    /** @brief When true, the view never shows anything outside @ref bounds. */
    bool clampToBounds;

    /** @brief World-space rectangle the view is kept inside when @ref clampToBounds is set. */
    Rectangle bounds;

    /** @brief Current shake intensity in [0, 1]. Set it to 1 to trigger a full shake. */
    float shakeTrauma;

    /** @brief Maximum shake offset in world units, reached when trauma is 1. */
    float shakeMagnitude;

    /** @brief Trauma lost per second. */
    float shakeDecay;

    /** @brief Shake offset of the current frame, written by @ref CameraSystem. */
    float shakeOffsetX = 0.0f;

    /** @brief Shake offset of the current frame, written by @ref CameraSystem. */
    float shakeOffsetY = 0.0f;

    /**
     * @brief Constructs a camera with explicit settings and defaults.
     * @param zoom Zoom factor.
     * @param primary Whether this camera can be used for rendering.
     */
    CameraComponent(float zoom = 1.0f, bool primary = true)
        : zoom(zoom), primary(primary), followTarget(std::nullopt), followSpeed(0.0f),
          clampToBounds(false), bounds{0.0f, 0.0f, 0.0f, 0.0f}, shakeTrauma(0.0f),
          shakeMagnitude(10.0f), shakeDecay(1.5f) {}
};
} // namespace Engine

#endif // ENGINE_COMPONENTS_CAMERACOMPONENT_HPP_
