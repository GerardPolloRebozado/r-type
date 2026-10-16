#include "Scripting/LuaAPI/Plugins/ParentAPI.hpp"
#include "Components/Parent.hpp"
#include "ECS/Entity.hpp"
#include "ECS/Registry.hpp"
#include "Logging/Logger.hpp"
#include "Plugins/HierarchyPlugin.hpp"
#include "Scripting/LuaAPI/Internal/Helpers.hpp"
#include "Systems/HierarchySystem.hpp"
#include <sol/sol.hpp>
#include <string>

namespace Engine::LuaAPI {

void registerParent(sol::state_view lua, Registry& registry) {
    // Idempotent: guarantees the Parent array exists even when the game never
    // ran HierarchyPlugin (Lua-first prototypes, editor preview scenes).
    registry.RegisterComponent<Parent>();

    sol::table component = Internal::ensureNamespace(lua, "Component");

    component["setParent"] = [&registry](Entity child, Entity parent,
                                         sol::optional<bool> relativeOpt) -> bool {
        if (!registry.IsEntityAlive(child) || !registry.IsEntityAlive(parent)) {
            return false;
        }
        try {
            HierarchyPlugin::AttachChild(registry, child, parent, relativeOpt.value_or(true));
            return true;
        } catch (const std::exception& e) {
            log_error(std::string("[LuaAPI] setParent failed: ") + e.what());
            return false;
        }
    };

    component["getParent"] = [lua, &registry](Entity entity) mutable -> sol::object {
        if (registry.HasComponent<Parent>(entity)) {
            return sol::make_object(lua, registry.GetComponent<Parent>(entity).parent);
        }
        return sol::make_object(lua, sol::lua_nil);
    };

    component["getChildren"] = [lua, &registry](Entity entity) mutable -> sol::table {
        sol::table out = lua.create_table();
        if (!registry.IsEntityAlive(entity)) {
            return out;
        }
        std::size_t i = 1;
        for (Entity child : HierarchySystem::FindDirectChildren(registry, entity)) {
            if (registry.IsEntityAlive(child)) {
                out[i++] = child;
            }
        }
        return out;
    };
}

} // namespace Engine::LuaAPI
