#include "UI/UiSystem.hpp"
#include "Assets/ProjectPaths.hpp"
#include "ECS/Registry.hpp"
#include "Logging/Logger.hpp"
#include "Scripting/ScriptSystem.hpp"
#include "UI/UiComponent.hpp"
#include <filesystem>
#include <memory>
#include <typeindex>
#include <vector>

namespace Engine {

std::unordered_set<std::string> UiSystem::s_WarnedMissing;

namespace {

// GetSystem throws when the script plugin was not loaded.
std::shared_ptr<ScriptSystem> FindScriptSystem(Registry& registry) {
    const auto& systems = registry.GetSystems();
    const auto it = systems.find(typeid(ScriptSystem));
    if (it == systems.end()) {
        return nullptr;
    }
    return std::static_pointer_cast<ScriptSystem>(it->second);
}

void CallHook(sol::table table, const std::string& name, Entity entity) {
    sol::optional<sol::function> hook = table[name];
    sol::protected_function_result result;

    if (!hook || !hook->valid()) {
        return;
    }
    result = hook->call(table, entity);
    if (!result.valid()) {
        sol::error err = result;
        log_user_error(std::string("[UI] ") + name + " error (entity " + std::to_string(entity) +
                       "): " + err.what());
    }
}

} // namespace

void UiSystem::Draw(Registry& registry) {
    std::shared_ptr<ScriptSystem> scripts = FindScriptSystem(registry);

    if (!scripts || !scripts->IsApiReady()) {
        return;
    }
    if (!registry.IsComponentRegistered<UiComponent>()) {
        return;
    }

    for (size_t i = 0; i < registry.GetEntityCount(); ++i) {
        Entity entity{i};
        sol::optional<sol::function> draw;

        if (!registry.IsEntityAlive(entity) || !registry.HasComponent<UiComponent>(entity)) {
            continue;
        }
        auto& ui = registry.RequireComponent<UiComponent>(entity);
        if (!ui.loaded && !ui.scriptPath.empty()) {
            LoadScript(registry, entity);
        }
        if (!ui.loaded || !ui.scriptTable.valid()) {
            continue;
        }
        draw = ui.scriptTable["draw"];
        if (!draw || !draw->valid()) {
            continue;
        }
        // One bad script must not skip the other entities this frame.
        auto result = draw->call(ui.scriptTable, entity);
        if (!result.valid()) {
            sol::error err = result;
            log_user_error(std::string("[UI] draw error (entity ") + std::to_string(entity) + "): " +
                           err.what());
        }
    }
}

void UiSystem::LoadScript(Registry& registry, Entity entity) {
    auto& ui = registry.RequireComponent<UiComponent>(entity);
    std::shared_ptr<ScriptSystem> scripts = FindScriptSystem(registry);
    std::filesystem::path fullPath;
    sol::protected_function_result result;
    sol::object object;

    if (!scripts || !scripts->IsApiReady() || ui.scriptPath.empty()) {
        return;
    }
    fullPath = ResolveAssetPath(registry, ui.scriptPath);
    if (!std::filesystem::exists(fullPath)) {
        // The path is checked every frame. Warn only the first time.
        if (s_WarnedMissing.insert(ui.scriptPath).second) {
            log_user_error("[UI] Script not found: " + fullPath.string());
        }
        return;
    }

    result = scripts->GetLua().script_file(fullPath.string());
    if (!result.valid()) {
        sol::error err = result;
        log_user_error("[UI] Failed to load script " + fullPath.string() + ": " + err.what());
        return;
    }
    object = result;
    if (object.get_type() != sol::type::table) {
        log_user_error("[UI] Script must return a table: " + fullPath.string());
        return;
    }

    ui.scriptTable = object.as<sol::table>();
    ui.loaded = true;
    // onOpen runs once, after the table exists.
    CallHook(ui.scriptTable, "onOpen", entity);
    log_info("[UI] Loaded UI script for entity " + std::to_string(entity) + ": " + ui.scriptPath);
}

void UiSystem::UnloadScript(Registry& registry, Entity entity) {
    auto& ui = registry.RequireComponent<UiComponent>(entity);

    if (!ui.loaded || !ui.scriptTable.valid()) {
        ui.scriptTable = sol::lua_nil;
        ui.loaded = false;
        return;
    }
    CallHook(ui.scriptTable, "onClose", entity);
    ui.scriptTable = sol::lua_nil;
    ui.loaded = false;
}

void UiSystem::OnSceneUnloaded(Registry& registry) {
    std::vector<Entity> entities;
    std::shared_ptr<ComponentArray<UiComponent>> components;

    if (!registry.IsComponentRegistered<UiComponent>()) {
        return;
    }
    // onClose can remove the component, so the list is copied first.
    components = registry.GetComponentArray<UiComponent>();
    for (const auto& [entity, ui] : *components) {
        if (ui.loaded) {
            entities.push_back(entity);
        }
    }
    for (const Entity entity : entities) {
        if (registry.HasComponent<UiComponent>(entity)) {
            UnloadScript(registry, entity);
        }
    }
}

} // namespace Engine
