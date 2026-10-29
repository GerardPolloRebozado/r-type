#include "Scripting/LuaAPI/Plugins/SpriteAPI.hpp"
#include "Assets/AssetManager.hpp"
#include "Assets/ProjectPaths.hpp"
#include "Components/Name.hpp"
#include "Components/SpriteComponent.hpp"
#include "ECS/Entity.hpp"
#include "ECS/Registry.hpp"
#include "Logging/Logger.hpp"
#include "Scripting/LuaAPI/Internal/Helpers.hpp"
#include <sol/sol.hpp>
#include <string>

namespace Engine::LuaAPI {

namespace {
bool loadSpriteTexture(Registry& registry, const std::string& path) {
    if (!registry.HasResource<AssetManager>() || path.empty()) {
        return false;
    }
    auto& am = registry.GetResource<AssetManager>();
    try {
        try {
            auto existing = am.GetSprite(path);
            if (existing && existing->GetTexture().id == 0) {
                am.UnloadSprite(path);
            }
        } catch (...) {
            // Not cached yet: fall through to LoadSprite
        }
        am.LoadSprite(path, ResolveAssetPath(registry, path).string());
        return true;
    } catch (const std::exception& e) {
        log_error(std::string("[LuaAPI] Sprite load failed for '") + path + "': " + e.what());
        return false;
    }
}
} // namespace

void registerSprite(sol::state_view lua, Registry& registry) {
    sol::table component = Internal::ensureNamespace(lua, "Component");

    component["addSprite"] =
        [&registry](Entity entity, const std::string& path, sol::optional<float> offsetXOpt,
                    sol::optional<float> offsetYOpt, sol::optional<float> scaleXOpt,
                    sol::optional<float> scaleYOpt) -> bool {
        if (!loadSpriteTexture(registry, path)) {
            return false;
        }
        auto& am = registry.GetResource<AssetManager>();
        auto spr = am.GetSprite(path);
        Internal::ensureTransform(registry, entity, true);
        registry.AddComponent<SpriteComponent>(
            entity, SpriteComponent{spr, path, offsetXOpt.value_or(0.0f), offsetYOpt.value_or(0.0f),
                                    scaleXOpt.value_or(1.0f), scaleYOpt.value_or(1.0f)});
        return true;
    };

    component["setSpriteRect"] = [&registry](Entity entity, float x, float y, float width,
                                             float height) -> bool {
        if (!registry.HasComponent<SpriteComponent>(entity)) {
            return false;
        }
        auto& sc = registry.RequireComponent<SpriteComponent>(entity);
        if (sc.sprite) {
            try {
                sc.sprite->SetSourceRect(raylib::Rectangle(x, y, width, height));
                return true;
            } catch (...) {
                return false;
            }
        }
        return false;
    };

    component["getSprite"] = [lua, &registry](Entity entity) mutable -> sol::object {
        if (registry.HasComponent<SpriteComponent>(entity)) {
            return sol::make_object(lua, "sprite");
        }
        return sol::make_object(lua, sol::lua_nil);
    };

    component["getSpriteInfo"] = [lua, &registry](Entity entity) mutable -> sol::object {
        if (!registry.HasComponent<SpriteComponent>(entity)) {
            return sol::make_object(lua, sol::lua_nil);
        }
        const auto& sc = registry.RequireComponent<SpriteComponent>(entity);
        sol::table t = lua.create_table();
        t["path"] = sc.assetPath;
        t["offsetX"] = sc.offsetX;
        t["offsetY"] = sc.offsetY;
        t["scaleX"] = sc.scaleX;
        t["scaleY"] = sc.scaleY;
        t["flipX"] = sc.flipX;
        t["flipY"] = sc.flipY;
        return sol::make_object(lua, t);
    };

    component["getSpriteFlip"] = [lua, &registry](Entity entity) mutable -> sol::object {
        if (!registry.HasComponent<SpriteComponent>(entity)) {
            return sol::make_object(lua, sol::lua_nil);
        }
        const auto& sc = registry.RequireComponent<SpriteComponent>(entity);
        sol::table t = lua.create_table();
        t["flipX"] = sc.flipX;
        t["flipY"] = sc.flipY;
        return sol::make_object(lua, t);
    };

    component["setSpriteFlip"] = [&registry](Entity entity, bool flipX,
                                             sol::optional<bool> flipYOpt) -> bool {
        if (!registry.HasComponent<SpriteComponent>(entity)) {
            return false;
        }
        auto& sc = registry.RequireComponent<SpriteComponent>(entity);
        sc.flipX = flipX;
        if (flipYOpt.has_value()) {
            sc.flipY = flipYOpt.value();
        }
        return true;
    };

    component["getSpriteScale"] = [lua, &registry](Entity entity) mutable -> sol::object {
        if (!registry.HasComponent<SpriteComponent>(entity)) {
            return sol::make_object(lua, sol::lua_nil);
        }
        const auto& sc = registry.RequireComponent<SpriteComponent>(entity);
        sol::table t = lua.create_table();
        t["x"] = sc.scaleX;
        t["y"] = sc.scaleY;
        return sol::make_object(lua, t);
    };

    component["getSpriteOffset"] = [lua, &registry](Entity entity) mutable -> sol::object {
        if (!registry.HasComponent<SpriteComponent>(entity)) {
            return sol::make_object(lua, sol::lua_nil);
        }
        const auto& sc = registry.RequireComponent<SpriteComponent>(entity);
        sol::table t = lua.create_table();
        t["x"] = sc.offsetX;
        t["y"] = sc.offsetY;
        return sol::make_object(lua, t);
    };

    component["getSpritePath"] = [lua, &registry](Entity entity) mutable -> sol::object {
        if (!registry.HasComponent<SpriteComponent>(entity)) {
            return sol::make_object(lua, sol::lua_nil);
        }
        return sol::make_object(lua, registry.RequireComponent<SpriteComponent>(entity).assetPath);
    };

    component["setSpriteScale"] = [&registry](Entity entity, float scaleX, float scaleY) -> bool {
        if (!registry.HasComponent<SpriteComponent>(entity)) {
            return false;
        }
        auto& sc = registry.RequireComponent<SpriteComponent>(entity);
        sc.scaleX = scaleX;
        sc.scaleY = scaleY;
        return true;
    };

    component["setSpriteOffset"] = [&registry](Entity entity, float offsetX,
                                               float offsetY) -> bool {
        if (!registry.HasComponent<SpriteComponent>(entity)) {
            return false;
        }
        auto& sc = registry.RequireComponent<SpriteComponent>(entity);
        sc.offsetX = offsetX;
        sc.offsetY = offsetY;
        return true;
    };

    component["setSpritePath"] = [&registry](Entity entity, const std::string& path) -> bool {
        if (!registry.HasComponent<SpriteComponent>(entity)) {
            return false;
        }
        if (!loadSpriteTexture(registry, path)) {
            return false;
        }
        auto& am = registry.GetResource<AssetManager>();
        auto& sc = registry.RequireComponent<SpriteComponent>(entity);
        sc.sprite = am.GetSprite(path);
        sc.assetPath = path;
        return true;
    };

    // Find an entity by its Name component value
    // not ideal though
    component["findEntityByName"] = [&registry, lua](const std::string& name) -> sol::object {
        for (std::size_t i = 0; i < registry.GetEntityCount(); ++i) {
            Entity entity = i;
            if (registry.HasComponent<Engine::Name>(entity)) {
                const auto& nameComp = registry.RequireComponent<Engine::Name>(entity);
                if (nameComp.value == name) {
                    return sol::make_object(lua, entity);
                }
            }
        }
        return sol::object{};
    };
}

} // namespace Engine::LuaAPI
