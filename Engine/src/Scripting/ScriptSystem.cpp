#include "Scripting/ScriptSystem.hpp"
#include "Assets/ProjectPaths.hpp"
#include "Core/EditorMode.hpp"
#include "ECS/Registry.hpp"
#include "Logging/Logger.hpp"
#include "Scripting/LuaAPI.hpp"
#include "Scripting/LuaBinder.hpp"
#include "Scripting/ScriptComponent.hpp"
#include <filesystem>
#include <vector>

namespace Engine {

ScriptSystem::ScriptSystem() = default;
ScriptSystem::~ScriptSystem() = default;

// Track which scripts we've already warned about (prevents log spam in Editor)
std::unordered_set<std::string> ScriptSystem::s_warnedMissingScripts;

void ScriptSystem::Update(Registry& registry, float dt) {
    if (!apiRegistered) {
        lua.open_libraries();
        LuaAPI::registerEngineAPI(lua.lua_state(), registry);
        // Plugins such as UI register here, once, after the engine API.
        // The editor needs this too, so a placed widget can be previewed.
        if (registry.HasResource<LuaBinder>()) {
            auto& modules = registry.GetResource<LuaBinder>().modules;
            for (size_t i = 0; i < modules.size(); ++i) {
                if (modules[i]) {
                    modules[i](lua, registry);
                }
            }
        }
        apiRegistered = true;
    }
    if (registry.HasResource<EditorMode>()) {
        return;
    }

    for (size_t i = 0; i < registry.GetEntityCount(); ++i) {
        Entity entity{i};
        if (!registry.HasComponent<ScriptComponent>(entity)) {
            continue;
        }

        auto& script = registry.RequireComponent<ScriptComponent>(entity);

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
                    log_user_error(std::string("[ScriptSystem] onUpdate error (entity ") +
                              std::to_string(entity) + "): " + err.what());
                }
            }
        }
    }
}

void ScriptSystem::loadScript(Registry& registry, Entity entity) {
    auto& script = registry.RequireComponent<ScriptComponent>(entity);
    if (script.scriptPath.empty()) {
        return;
    }

    std::filesystem::path fullPath = ResolveAssetPath(registry, script.scriptPath);

    if (!std::filesystem::exists(fullPath)) {
        // Only warn once per missing script path to avoid log spam
        if (s_warnedMissingScripts.insert(script.scriptPath).second) {
            log_user_error("[ScriptSystem] Script not found: " + fullPath.string());
        }
        return;
    }

    auto result = lua.script_file(fullPath.string(), &sol::script_default_on_error);
    if (!result.valid()) {
        sol::error err = result;
        log_user_error("[ScriptSystem] Failed to load script " + fullPath.string() + ": " +
                       err.what());
        return;
    }

    sol::object obj = result;
    if (obj.get_type() != sol::type::table) {
        log_user_error("[ScriptSystem] Script must return a table: " + fullPath.string());
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
            log_user_error(std::string("[ScriptSystem] onCreate error (entity ") +
                      std::to_string(entity) + "): " + err.what());
        }
    }

    log_info("[ScriptSystem] Loaded script for entity " + std::to_string(entity) + ": " +
             script.scriptPath);
}

void ScriptSystem::unloadScript(Registry& registry, Entity entity) {
    auto& script = registry.RequireComponent<ScriptComponent>(entity);
    if (script.loaded && script.scriptTable.valid()) {
        sol::optional<sol::function> destroy = script.scriptTable["onDestroy"];
        if (destroy && destroy->valid()) {
            // Pass the module table as self, like onCreate/onUpdate, so
            // function Script:onDestroy(entity) receives the entity.
            auto result = destroy->call(script.scriptTable, entity);
            if (!result.valid()) {
                sol::error err = result;
                log_user_error(std::string("[ScriptSystem] onDestroy error (entity ") +
                          std::to_string(entity) + "): " + err.what());
            }
        }
        script.scriptTable = sol::lua_nil;
        script.loaded = false;
    }
}

sol::state_view ScriptSystem::GetLua() {
    return lua;
}

bool ScriptSystem::IsApiReady() const {
    return apiRegistered;
}

void ScriptSystem::OnSceneUnloaded(Registry& registry) {
    if (!registry.IsComponentRegistered<ScriptComponent>()) {
        return;
    }
    // Collect first: onDestroy may add or remove components.
    std::vector<Entity> scripted;
    for (const auto& [entity, script] : *registry.GetComponentArray<ScriptComponent>()) {
        if (script.loaded) {
            scripted.push_back(entity);
        }
    }
    for (const Entity entity : scripted) {
        if (registry.HasComponent<ScriptComponent>(entity)) {
            unloadScript(registry, entity);
        }
    }
}

} // namespace Engine
