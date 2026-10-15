#ifndef ENGINE_SCRIPTING_LUAAPI_SPRITEAPI_HPP_
#define ENGINE_SCRIPTING_LUAAPI_SPRITEAPI_HPP_

namespace sol {
class state_view;
}
namespace Engine {
class Registry;
}

namespace Engine::LuaAPI {
/**
 * @brief Registers the sprite bindings in the `Component` namespace.
 *        Mirrors Engine's SpritePlugin / SpriteComponent.
 *
 * Getters return nil when the entity has no SpriteComponent; setters return
 * false in that case (setSpritePath also returns false when the texture
 * cannot be loaded). Paths are resolved like everywhere else in the engine: project-relative in
 * built games (whose working directory is the project folder), absolute otherwise.
 *
 * @param lua View of the shared Lua state (lambdas capture a copy by value).
 * @param registry ECS registry; also owns the AssetManager resource.
 */
void registerSprite(sol::state_view lua, Registry& registry);
} // namespace Engine::LuaAPI

#endif // ENGINE_SCRIPTING_LUAAPI_SPRITEAPI_HPP_
