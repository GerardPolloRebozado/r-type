#ifndef ENGINE_CORE_UIPASS_HPP_
#define ENGINE_CORE_UIPASS_HPP_

#include "ECS/Registry.hpp"
#include <functional>

namespace Engine {

/**
 * @brief Optional overlay drawn in window space after the letterboxed canvas.
 *
 * The game branch of Application::PostUpdate calls @ref draw inside
 * BeginDrawing. The editor's custom render path does not. Headless frames
 * never reach it.
 */
struct UiPass {
    std::function<void(Registry&)> draw;
};

} // namespace Engine

#endif // ENGINE_CORE_UIPASS_HPP_
