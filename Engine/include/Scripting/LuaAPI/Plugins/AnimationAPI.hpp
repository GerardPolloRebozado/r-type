#ifndef ENGINE_SCRIPTING_LUAAPI_ANIMATIONAPI_HPP_
#define ENGINE_SCRIPTING_LUAAPI_ANIMATIONAPI_HPP_

namespace sol {
class state_view;
}
namespace Engine {
class Registry;
}

namespace Engine::LuaAPI {
/**
 * @brief Registers the animation bindings in the `Component` namespace.
 *        Mirrors Engine's AnimationPlugin / AnimationComponent.
 *
 * Getters return nil (or false) when the entity has no Animation component.
 * The clip state is advanced by AnimationSystem; scripts only pick a clip and
 * toggle playback.
 *
 *   - `Component.addAnimation(entity, pathsOrSheet [, frameWidth, frameHeight,
 *     totalFrames, frameDuration, loop])` -> bool
 *   - `Component.playAnimation(entity, clipName [, restartIfSame])` -> bool
 *   - `Component.getAnimation(entity)` -> clip name or nil
 *   - `Component.getAnimationClips(entity)` -> `{name1, name2, ...}` or nil
 *   - `Component.setAnimationPlaying(entity, playing)` -> bool
 *   - `Component.isAnimationPlaying(entity)` -> bool
 *   - `Component.getAnimationFrame(entity)` -> frame index or nil
 *   - `Component.setAnimationFlip(entity, flipX [, flipY])` -> bool
 *   - `Component.getAnimationFlip(entity)` -> `{flipX = bool, flipY = bool}` or nil
 *
 * `pathsOrSheet` is either a table of frame image paths (one clip from
 * multiple files) or a single spritesheet path sliced with `frameWidth` and
 * `frameHeight`.
 *
 * @param lua View of the shared Lua state.
 * @param registry ECS registry; also owns the AssetManager resource.
 */
void registerAnimation(sol::state_view lua, Registry& registry);
} // namespace Engine::LuaAPI

#endif // ENGINE_SCRIPTING_LUAAPI_ANIMATIONAPI_HPP_
