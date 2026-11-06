#ifndef ENGINE_SCRIPTING_LUAAPI_PLUGINS_ANIMATIONAPI_HPP_
#define ENGINE_SCRIPTING_LUAAPI_PLUGINS_ANIMATIONAPI_HPP_

namespace sol {
class state_view;
}

namespace Engine {
class Registry;
}

namespace Engine::LuaAPI {
/**
 * @brief Registers animation bindings in the `Animation` / `Component` namespace.
 *        Mirrors Engine's AnimationPlugin / Animation component.
 *
 * Exposes:
 *   - `Animation.play(entity, clipName [, restartIfSame])` -> bool
 *   - `Animation.getCurrent(entity)` -> string or nil
 *   - `Animation.isPlaying(entity)` -> bool
 *   - `Animation.pause(entity)` -> bool
 *   - `Animation.resume(entity)` -> bool
 *   - `Animation.setFlip(entity, flipX [, flipY])` -> bool
 *   - `Animation.getFlip(entity)` -> {flipX, flipY} table or nil
 *   - `Component.playAnimation(entity, clipName [, restartIfSame])` -> bool
 *
 * @param lua View of the shared Lua state.
 * @param registry ECS registry holding the entities and Animation components.
 */
void registerAnimation(sol::state_view lua, Registry& registry);
} // namespace Engine::LuaAPI

#endif // ENGINE_SCRIPTING_LUAAPI_PLUGINS_ANIMATIONAPI_HPP_
