#include "Scripting/LuaAPI/Plugins/ScriptAPI.hpp"
#include "ECS/Entity.hpp"
#include "ECS/Registry.hpp"
#include "Logging/Logger.hpp"
#include "Scripting/LuaAPI/Internal/Helpers.hpp"
#include "Scripting/ScriptComponent.hpp"
#include <sol/sol.hpp>
#include <string>

namespace Engine::LuaAPI {

void registerScript(const sol::state_view& lua, Registry& registry) {
    registry.RegisterComponent<ScriptComponent>();

    sol::table component = Internal::ensureNamespace(lua, "Component");

    component["addScript"] = [&registry](Entity entity, const std::string& path,
                                         sol::optional<bool> autoReloadOpt) -> bool {
        if (!registry.IsEntityAlive(entity)) {
            log_error("[LuaAPI] addScript: dead entity");
            return false;
        }
        if (path.empty()) {
            log_error("[LuaAPI] addScript: empty script path");
            return false;
        }
        const bool autoReload = autoReloadOpt.value_or(true);
        if (registry.HasComponent<ScriptComponent>(entity)) {
            auto& script = registry.GetComponent<ScriptComponent>(entity);
            if (script.scriptPath == path) {
                return true;
            }
            script = ScriptComponent(path, autoReload);
            return true;
        }
        registry.AddComponent<ScriptComponent>(entity, ScriptComponent(path, autoReload));
        return true;
    };

    component["getScriptPath"] = [lua, &registry](Entity entity) mutable -> sol::object {
        if (registry.HasComponent<ScriptComponent>(entity)) {
            const std::string& path = registry.GetComponent<ScriptComponent>(entity).scriptPath;
            if (!path.empty()) {
                return sol::make_object(lua, path);
            }
        }
        return sol::make_object(lua, sol::lua_nil);
    };
}

} // namespace Engine::LuaAPI