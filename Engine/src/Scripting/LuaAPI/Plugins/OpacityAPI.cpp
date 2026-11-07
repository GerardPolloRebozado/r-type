#include "Scripting/LuaAPI/Plugins/OpacityAPI.hpp"
#include "Components/Opacity.hpp"
#include "ECS/Entity.hpp"
#include "ECS/Registry.hpp"
#include "Scripting/LuaAPI/Internal/Helpers.hpp"
#include <algorithm>
#include <sol/sol.hpp>

namespace Engine::LuaAPI {

void registerOpacity(sol::state_view lua, Registry& registry) {
    // Idempotent: guarantees the Opacity array exists even when the game never
    // ran OpacityPlugin (Lua-first prototypes, editor preview scenes).
    registry.RegisterComponent<Opacity>();

    sol::table component = Internal::ensureNamespace(lua, "Component");

    component["setOpacity"] = [&registry](Entity entity, float alpha,
                                          sol::optional<bool> inheritOpt) -> bool {
        if (!registry.IsEntityAlive(entity)) {
            return false;
        }
        const float clamped = std::clamp(alpha, 0.0f, 1.0f);
        if (inheritOpt.has_value()) {
            registry.AddComponent<Opacity>(entity, Opacity::Create(clamped, inheritOpt.value()));
        } else if (registry.HasComponent<Opacity>(entity)) {
            // Preserve the existing inherit flag when only alpha is set.
            registry.RequireComponent<Opacity>(entity).alpha = clamped;
        } else {
            registry.AddComponent<Opacity>(entity, Opacity::Create(clamped));
        }
        return true;
    };

    component["setOpacityInherit"] = [&registry](Entity entity, bool inherit) -> bool {
        if (!registry.HasComponent<Opacity>(entity)) {
            return false;
        }
        registry.RequireComponent<Opacity>(entity).inheritToChildren = inherit;
        return true;
    };

    component["getOpacityInherit"] = [luaState = lua.lua_state(),
                                      &registry](Entity entity) mutable -> sol::object {
        sol::state_view lua(luaState);
        if (registry.HasComponent<Opacity>(entity)) {
            return sol::make_object(lua,
                                    registry.RequireComponent<Opacity>(entity).inheritToChildren);
        }
        return sol::make_object(lua, sol::lua_nil);
    };

    component["getOpacity"] = [luaState = lua.lua_state(),
                               &registry](Entity entity) mutable -> sol::object {
        sol::state_view lua(luaState);
        if (registry.HasComponent<Opacity>(entity)) {
            return sol::make_object(lua, registry.RequireComponent<Opacity>(entity).alpha);
        }
        return sol::make_object(lua, sol::lua_nil);
    };
}

} // namespace Engine::LuaAPI
