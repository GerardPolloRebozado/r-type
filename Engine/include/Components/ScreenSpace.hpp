#ifndef ENGINE_COMPONENTS_SCREENSPACE_HPP_
#define ENGINE_COMPONENTS_SCREENSPACE_HPP_

namespace Engine {
/**
 * @struct ScreenSpace
 * @brief Tag component: the entity is drawn in screen pixels, ignoring the camera.
 *
 * Used for HUD elements that must stay fixed while the
 * world scrolls.
 */
struct ScreenSpace {};
} // namespace Engine

#endif // ENGINE_COMPONENTS_SCREENSPACE_HPP_
