#include "Scripting/LuaAPI/Plugins/SpriteAPI.hpp"
#include "Assets/AssetManager.hpp"
#include "Components/SpriteComponent.hpp"
#include "ECS/Entity.hpp"
#include "ECS/Registry.hpp"
#include "Scripting/LuaAPI/Internal/Helpers.hpp"
#include <sol/sol.hpp>
#include <string>

namespace Engine::LuaAPI {

void registerSprite(sol::state_view lua, Registry& registry) {
    sol::table component = Internal::ensureNamespace(lua, "Component");

    component["addSprite"] = [&registry](Entity entity, const std::string& path, float offsetX,
                                         float offsetY, float scaleX, float scaleY) {
        if (!registry.HasResource<AssetManager>()) {
            return;
        }
        auto& am = registry.GetResource<AssetManager>();
        am.LoadSprite(path, path); // alias == path for scripts
        auto spr = am.GetSprite(path);
        Internal::ensureTransform(registry, entity, true);
        registry.AddComponent<SpriteComponent>(
            entity, SpriteComponent{spr, path, offsetX, offsetY, scaleX, scaleY});
    };

    component["getSprite"] = [lua, &registry](Entity entity) mutable -> sol::object {
        if (registry.HasComponent<SpriteComponent>(entity)) {
            return sol::make_object(lua, "sprite");
        }
        return sol::make_object(lua, sol::lua_nil);
    };
}

} // namespace Engine::LuaAPI
