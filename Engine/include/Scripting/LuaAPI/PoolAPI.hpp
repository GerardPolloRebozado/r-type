#ifndef ENGINE_SCRIPTING_LUA_POOLAPI_HPP_
#define ENGINE_SCRIPTING_LUA_POOLAPI_HPP_

#include "ECS/Registry.hpp"
#include <sol/sol.hpp>

namespace Engine::LuaAPI {
void registerPool(sol::state_view lua, Registry& registry);
}

#endif