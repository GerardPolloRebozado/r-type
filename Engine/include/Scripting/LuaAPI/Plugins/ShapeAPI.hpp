#ifndef ENGINE_SCRIPTING_LUAAPI_SHAPEAPI_HPP_
#define ENGINE_SCRIPTING_LUAAPI_SHAPEAPI_HPP_

namespace sol {
class state_view;
}
namespace Engine {
class Registry;
}

namespace Engine::LuaAPI {
/**
 * @brief Registers `Component.addShape`.
 *        Mirrors Engine's ShapePlugin / ShapeComponent.
 *
 * @param lua View of the shared Lua state (lambdas capture a copy by value).
 * @param registry ECS registry used to add the Shape component.
 */
void registerShape(sol::state_view lua, Registry& registry);
} // namespace Engine::LuaAPI

#endif // ENGINE_SCRIPTING_LUAAPI_SHAPEAPI_HPP_