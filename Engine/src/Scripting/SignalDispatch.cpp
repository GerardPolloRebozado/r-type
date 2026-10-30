#include "Scripting/SignalDispatch.hpp"
#include "Components/SignalConnections.hpp"
#include "Core/EditorMode.hpp"
#include "ECS/Registry.hpp"
#include "Logging/Logger.hpp"
#include "Scripting/ScriptComponent.hpp"
#include <sol/sol.hpp>
#include <string>
#include <unordered_set>

namespace Engine::Signals {

namespace {

std::unordered_set<std::string>& WarnedKeys() {
    static std::unordered_set<std::string> keys;
    return keys;
}

void WarnOnce(const std::string& key, const std::string& message) {
    if (WarnedKeys().insert(key).second) {
        log_user_error(message);
    }
}

} // namespace

std::vector<SignalProvider> GetSignalProviders() {
    std::vector<SignalProvider> providers;
    providers.push_back(
        SignalProvider{"Collider", std::vector<std::string_view>(kCollisionSignals.begin(),
                                                                 kCollisionSignals.end())});
    return providers;
}

void EmitSignal(Registry& registry, Entity emitter, const std::string& signal, Entity other) {
    if (registry.HasResource<EditorMode>()) {
        return;
    }
    if (!registry.IsComponentRegistered<SignalConnections>() ||
        !registry.HasComponent<SignalConnections>(emitter)) {
        return;
    }
    const std::vector<SignalConnection> connections =
        registry.RequireComponent<SignalConnections>(emitter).connections;
    for (const auto& connection : connections) {
        if (connection.signal != signal) {
            continue;
        }
        if (!registry.IsEntityAlive(connection.target)) {
            continue;
        }
        if (!registry.HasComponent<ScriptComponent>(connection.target)) {
            WarnOnce("noscript:" + std::to_string(connection.target),
                     "[Signals] Connection target " + std::to_string(connection.target) +
                         " has no script; skipping '" + connection.method + "'");
            continue;
        }
        auto& script = registry.RequireComponent<ScriptComponent>(connection.target);
        if (!script.loaded || !script.scriptTable.valid()) {
            continue;
        }
        sol::optional<sol::function> fn = script.scriptTable[connection.method];
        if (!fn || !fn->valid()) {
            WarnOnce("nomethod:" + std::to_string(connection.target) + ":" + connection.method,
                     "[Signals] Target " + std::to_string(connection.target) + " has no method '" +
                         connection.method + "'; skipping");
            continue;
        }
        auto result = fn->call(script.scriptTable, emitter, other);
        if (!result.valid()) {
            sol::error err = result;
            log_user_error("[Signals] '" + signal + "' handler '" + connection.method +
                      "' error (target " + std::to_string(connection.target) + "): " + err.what());
        }
    }
}

} // namespace Engine::Signals
