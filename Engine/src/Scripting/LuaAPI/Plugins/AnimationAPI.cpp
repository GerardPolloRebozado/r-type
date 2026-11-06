#include "Scripting/LuaAPI/Plugins/AnimationAPI.hpp"
#include "Components/Animation.hpp"
#include "ECS/Entity.hpp"
#include "ECS/Registry.hpp"
#include "Scripting/LuaAPI/Internal/Helpers.hpp"
#include <sol/sol.hpp>

namespace Engine::LuaAPI {

void registerAnimation(sol::state_view lua, Registry& registry) {
    if (!registry.IsComponentRegistered<Animation>()) {
        registry.RegisterComponent<Animation>();
    }

    sol::table animation = Internal::ensureNamespace(lua, "Animation");
    sol::table component = Internal::ensureNamespace(lua, "Component");

    auto playFunc = [&registry](Entity entity, const std::string& clipName,
                                sol::optional<bool> restartIfSame) -> bool {
        if (!registry.HasComponent<Animation>(entity)) {
            return false;
        }
        auto& anim = registry.RequireComponent<Animation>(entity);
        return anim.Play(clipName, restartIfSame.value_or(false));
    };

    animation["play"] = playFunc;
    component["playAnimation"] = playFunc;

    animation["getCurrent"] = [lua, &registry](Entity entity) mutable -> sol::object {
        if (!registry.HasComponent<Animation>(entity)) {
            return sol::make_object(lua, sol::lua_nil);
        }
        const auto& anim = registry.RequireComponent<Animation>(entity);
        return sol::make_object(lua, anim.currentAnimation);
    };

    animation["isPlaying"] = [&registry](Entity entity) -> bool {
        if (!registry.HasComponent<Animation>(entity)) {
            return false;
        }
        return registry.RequireComponent<Animation>(entity).isPlaying;
    };

    animation["pause"] = [&registry](Entity entity) -> bool {
        if (!registry.HasComponent<Animation>(entity)) {
            return false;
        }
        registry.RequireComponent<Animation>(entity).isPlaying = false;
        return true;
    };

    animation["resume"] = [&registry](Entity entity) -> bool {
        if (!registry.HasComponent<Animation>(entity)) {
            return false;
        }
        registry.RequireComponent<Animation>(entity).isPlaying = true;
        return true;
    };

    animation["setFlip"] = [&registry](Entity entity, bool flipX,
                                       sol::optional<bool> flipY) -> bool {
        if (!registry.HasComponent<Animation>(entity)) {
            return false;
        }
        auto& anim = registry.RequireComponent<Animation>(entity);
        anim.flipX = flipX;
        if (flipY.has_value()) {
            anim.flipY = flipY.value();
        }
        return true;
    };

    animation["getFlip"] = [lua, &registry](Entity entity) mutable -> sol::object {
        if (!registry.HasComponent<Animation>(entity)) {
            return sol::make_object(lua, sol::lua_nil);
        }
        const auto& anim = registry.RequireComponent<Animation>(entity);
        sol::table t = lua.create_table();
        t["flipX"] = anim.flipX;
        t["flipY"] = anim.flipY;
        return sol::make_object(lua, t);
    };
}

} // namespace Engine::LuaAPI
