#include "Scripting/ScriptSystem.hpp"
#include "ECS/Registry.hpp"
#include "Logging/Logger.hpp"
#include "Resources/ProjectContext.hpp"
#include "Scripting/LuaAPI.hpp"
#include "Scripting/ScriptComponent.hpp"
#include <filesystem>

namespace Engine {

ScriptSystem::ScriptSystem() = default;
ScriptSystem::~ScriptSystem() = default;

// Track which scripts we've already warned about (prevents log spam in Editor)
std::unordered_set<std::string> ScriptSystem::s_warnedMissingScripts;

void ScriptSystem::Update(Registry& registry, float dt) {
    if (!apiRegistered) {
        lua.open_libraries();
        LuaAPI::registerEngineAPI(lua.lua_state(), registry);
        apiRegistered = true;
    }

    for (size_t i = 0; i < registry.GetEntityCount(); ++i) {
        Entity entity{i};
        if (!registry.HasComponent<ScriptComponent>(entity)) {
            continue;
        }

        auto& script = registry.GetComponent<ScriptComponent>(entity);

        // Load script on first encounter.
        if (!script.loaded && !script.scriptPath.empty()) {
            loadScript(registry, entity);
        }

        // Call onUpdate(dt) each frame if loaded.
        if (script.loaded && script.scriptTable.valid()) {
            sol::optional<sol::function> update = script.scriptTable["onUpdate"];
            if (update && update->valid()) {
                // Pass the module table as self so function Player:onUpdate() works
                auto result = update->call(script.scriptTable, entity, dt);
                if (!result.valid()) {
                    sol::error err = result;
                    log_error(std::string("[ScriptSystem] onUpdate error (entity ") +
                              std::to_string(entity) + "): " + err.what());
                }
            }
        }
    }
}

void ScriptSystem::loadScript(Registry& registry, Entity entity) {
    auto& script = registry.GetComponent<ScriptComponent>(entity);
    if (script.scriptPath.empty()) {
        return;
    }

    std::filesystem::path fullPath = ResolveProjectPath(registry, script.scriptPath);

    if (!std::filesystem::exists(fullPath)) {
        // Only warn once per missing script path to avoid log spam
        if (s_warnedMissingScripts.insert(script.scriptPath).second) {
            log_error("[ScriptSystem] Script not found: " + fullPath.string());
        }
        return;
    }

    auto result = lua.script_file(fullPath.string(), &sol::script_default_on_error);
    if (!result.valid()) {
        sol::error err = result;
        log_error("[ScriptSystem] Failed to load script " + fullPath.string() + ": " + err.what());
        return;
    }

    sol::object obj = result;
    if (obj.get_type() != sol::type::table) {
        log_error("[ScriptSystem] Script must return a table: " + fullPath.string());
        return;
    }

    script.scriptTable = obj.as<sol::table>();
    script.loaded = true;

    // Call onCreate if it exists
    sol::optional<sol::function> create = script.scriptTable["onCreate"];
    if (create && create->valid()) {
        // Pass the module table as self so function Player:onCreate() works.
        auto callResult = create->call(script.scriptTable, entity);
        if (!callResult.valid()) {
            sol::error err = callResult;
            log_error(std::string("[ScriptSystem] onCreate error (entity ") +
                      std::to_string(entity) + "): " + err.what());
        }
    }

    log_info("[ScriptSystem] Loaded script for entity " + std::to_string(entity) + ": " +
             script.scriptPath);
}

void ScriptSystem::unloadScript(Registry& registry, Entity entity) {
    auto& script = registry.GetComponent<ScriptComponent>(entity);
    if (script.loaded && script.scriptTable.valid()) {
        sol::optional<sol::function> destroy = script.scriptTable["onDestroy"];
        if (destroy && destroy->valid()) {
            auto result = destroy->call(entity);
            if (!result.valid()) {
                sol::error err = result;
                log_error(std::string("[ScriptSystem] onDestroy error (entity ") +
                          std::to_string(entity) + "): " + err.what());
            }
        }
        script.scriptTable = sol::lua_nil;
        script.loaded = false;
    }
}

} // namespace Engine
