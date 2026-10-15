#ifndef ENGINE_SCRIPTING_LUAAPI_ENTITYAPI_HPP_
#define ENGINE_SCRIPTING_LUAAPI_ENTITYAPI_HPP_

namespace sol {
class state_view;
}
namespace Engine {
class Registry;
}

namespace Engine::LuaAPI {
/**
 * @brief Registers the `Entity` namespace: create/destroy/find/getName/setName.
 *
 * @param lua View of the shared Lua state (lambdas capture a copy by value).
 * @param registry ECS registry used for entity creation, lookup, and Name.
 */
void registerEntity(sol::state_view lua, Registry& registry);
} // namespace Engine::LuaAPI

#endif // ENGINE_SCRIPTING_LUAAPI_ENTITYAPI_HPP_
