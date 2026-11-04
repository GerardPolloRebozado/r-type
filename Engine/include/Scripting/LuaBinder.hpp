#ifndef ENGINE_SCRIPTING_LUABINDER_HPP_
#define ENGINE_SCRIPTING_LUABINDER_HPP_

#include "ECS/Registry.hpp"
#include <functional>
#include <sol/sol.hpp>
#include <vector>

namespace Engine {

/**
 * @brief Extra Lua modules registered on the shared script state.
 *
 * ScriptSystem runs every module once, after the engine API, on its first
 * update. The state is owned by ScriptSystem and is not owned here.
 */
struct LuaBinder {
    std::vector<std::function<void(sol::state_view, Registry&)>> modules;
};

} // namespace Engine

#endif // ENGINE_SCRIPTING_LUABINDER_HPP_
