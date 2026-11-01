#ifndef ENGINE_SCRIPTING_LUAAPI_CAMERAAPI_HPP_
#define ENGINE_SCRIPTING_LUAAPI_CAMERAAPI_HPP_

#include <sol/sol.hpp>

namespace Engine {
class Registry;

namespace LuaAPI {
void registerCamera(sol::state_view lua, Registry& registry);
}
} // namespace Engine

#endif // ENGINE_SCRIPTING_LUAAPI_CAMERAAPI_HPP_
