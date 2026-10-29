#ifndef ENGINE_SCRIPTING_LUAAPI_PLUGINS_AUDIOAPI_HPP_
#define ENGINE_SCRIPTING_LUAAPI_PLUGINS_AUDIOAPI_HPP_

namespace sol {
class state_view;
}

namespace Engine {
class Registry;
}

namespace Engine::LuaAPI {
void registerAudio(sol::state_view lua, Registry& registry);
} // namespace Engine::LuaAPI

#endif // ENGINE_SCRIPTING_LUAAPI_PLUGINS_AUDIOAPI_HPP_
