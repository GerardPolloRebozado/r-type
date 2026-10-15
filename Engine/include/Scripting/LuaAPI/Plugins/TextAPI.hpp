#ifndef ENGINE_SCRIPTING_LUAAPI_TEXTAPI_HPP_
#define ENGINE_SCRIPTING_LUAAPI_TEXTAPI_HPP_

namespace sol {
class state_view;
}
namespace Engine {
class Registry;
}

namespace Engine::LuaAPI {
/**
 * @brief Registers `Component.addText`.
 *        Mirrors Engine's TextPlugin / Text component.
 *
 * @param lua View of the shared Lua state (lambdas capture a copy by value).
 * @param registry ECS registry used to add the Text component.
 */
void registerText(sol::state_view lua, Registry& registry);
} // namespace Engine::LuaAPI

#endif // ENGINE_SCRIPTING_LUAAPI_TEXTAPI_HPP_