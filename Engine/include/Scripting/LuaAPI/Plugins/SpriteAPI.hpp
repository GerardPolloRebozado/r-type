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
 * @brief Registers `Component.addSprite` and `Component.getSprite`.
 *        Mirrors Engine's SpritePlugin / SpriteComponent.
 *
 * @param lua View of the shared Lua state (lambdas capture a copy by value).
 * @param registry ECS registry; also owns the AssetManager resource.
 */
void registerSprite(sol::state_view lua, Registry& registry);
} // namespace Engine::LuaAPI

#endif // ENGINE_SCRIPTING_LUAAPI_SPRITEAPI_HPP_