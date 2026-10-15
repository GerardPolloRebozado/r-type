#ifndef ENGINE_SCRIPTING_LUAAPI_PLUGINS_OPACITYAPI_HPP_
#define ENGINE_SCRIPTING_LUAAPI_PLUGINS_OPACITYAPI_HPP_

namespace sol {
class state_view;
}
namespace Engine {
class Registry;
}

namespace Engine::LuaAPI {
/**
 * @brief Registers opacity bindings in the `Component` namespace.
 *        Mirrors Engine's OpacityPlugin / Opacity component.
 *
 *   - `Component.setOpacity(entity, alpha [, inherit])` -> bool (creates or
 *     updates, alpha clamped to [0, 1])
 *   - `Component.getOpacity(entity)` -> alpha number or nil
 *   - `Component.setOpacityInherit(entity, inherit)` / get flag for the
 *     "inherit to children" checkbox
 *
 * Removal uses the existing generic `Component.remove(entity, "Opacity")`.
 *
 * @param lua View of the shared Lua state (lambdas capture a copy by value).
 * @param registry ECS registry holding the entities.
 */
void registerOpacity(sol::state_view lua, Registry& registry);
} // namespace Engine::LuaAPI

#endif // ENGINE_SCRIPTING_LUAAPI_PLUGINS_OPACITYAPI_HPP_
