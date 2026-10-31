#ifndef WAVE_API_HPP_
#define WAVE_API_HPP_
#include "ECS/Registry.hpp"
#include <sol/sol.hpp>

namespace Engine {

/**
 * Registers the Wave API in the provided Lua state.
 * @param lua The sol::state_view representing the Lua interpreter.
 * @param registry Reference to the Engine registry for component access.
 */
void registerWave(sol::state_view lua, Registry& registry);
#endif

} // namespace Engine