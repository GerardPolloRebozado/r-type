#include "Scripting/ScriptVariableSync.hpp"

#include "ECS/Registry.hpp"
#include "Resources/ScriptVariableState.hpp"
#include "Scripting/LuaAPI/Internal/LuaValue.hpp"
#include "Scripting/ScriptComponent.hpp"

#include <set>

namespace Engine::Multiplayer {

bool ReadScriptVariables(Registry& registry, Entity entity, const std::vector<std::string>& fields,
                         glz::generic::object_t& out) {
    if (!registry.IsComponentRegistered<ScriptComponent>() ||
        !registry.HasComponent<ScriptComponent>(entity)) {
        return false;
    }
    auto& script = registry.RequireComponent<ScriptComponent>(entity);
    if (!script.loaded || !script.scriptTable.valid()) {
        return false;
    }

    for (const std::string& field : fields) {
        const sol::object value = script.scriptTable.get<sol::object>(field);
        if (!value.valid() || value.get_type() == sol::type::lua_nil ||
            value.get_type() == sol::type::none) {
            continue;
        }
        std::set<const void*> seen;
        out[field] = LuaAPI::Internal::LuaToGeneric(value, seen, 0);
    }
    return true;
}

bool WriteScriptVariables(Registry& registry, Entity entity, const glz::generic::object_t& vars) {
    if (!registry.IsComponentRegistered<ScriptComponent>() ||
        !registry.HasComponent<ScriptComponent>(entity)) {
        return false;
    }
    auto& script = registry.RequireComponent<ScriptComponent>(entity);
    if (!script.loaded || !script.scriptTable.valid()) {
        return false;
    }

    sol::state_view lua(script.scriptTable.lua_state());
    for (const auto& [field, value] : vars) {
        script.scriptTable[field] = LuaAPI::Internal::GenericToLua(lua, value);
    }
    return true;
}

void ApplyReplicatedScriptVariables(Registry& registry, std::size_t networkId, Entity entity,
                                    const glz::generic::object_t& vars) {
    if (!registry.HasResource<ScriptVariableState>()) {
        registry.AddResource<ScriptVariableState>();
    }
    auto& state = registry.GetResource<ScriptVariableState>();

    if (state.Find(entity) != nullptr && WriteScriptVariables(registry, entity, vars)) {
        state.RemovePending(networkId);
        return;
    }
    state.pendingByNetworkId[networkId] = vars;
}

void ResolvePendingScriptVariables(Registry& registry, Entity entity, std::size_t networkId) {
    if (!registry.HasResource<ScriptVariableState>()) {
        return;
    }
    auto& state = registry.GetResource<ScriptVariableState>();
    const auto it = state.pendingByNetworkId.find(networkId);
    if (it == state.pendingByNetworkId.end()) {
        return;
    }
    if (WriteScriptVariables(registry, entity, it->second)) {
        state.pendingByNetworkId.erase(it);
    }
}

} // namespace Engine::Multiplayer
