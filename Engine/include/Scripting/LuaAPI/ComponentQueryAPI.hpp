#ifndef ENGINE_SCRIPTING_LUAAPI_COMPONENTQUERYAPI_HPP_
#define ENGINE_SCRIPTING_LUAAPI_COMPONENTQUERYAPI_HPP_

namespace sol {
class state_view;
}
namespace Engine {
class Registry;
}

namespace Engine::LuaAPI {
/**
 * @brief Registers the cross-component query half of the `Component` namespace:
 *        `Component.has(entity, tag)` and `Component.remove(entity, tag)`.
 *
 * These two functions dispatch across every registered component type, so they
 * deliberately live in their own file referred to by every component module.
 *
 * @param lua View of the shared Lua state (lambdas capture a copy by value).
 * @param registry ECS registry used to query and remove components.
 */
void registerComponentQuery(sol::state_view lua, Registry& registry);
} // namespace Engine::LuaAPI

#endif // ENGINE_SCRIPTING_LUAAPI_COMPONENTQUERYAPI_HPP_
