#include "Scripting/LuaAPI/Plugins/ConnectionsAPI.hpp"
#include "Components/SignalConnections.hpp"
#include "ECS/Entity.hpp"
#include "ECS/Registry.hpp"
#include "Logging/Logger.hpp"
#include "Scripting/LuaAPI/Internal/Helpers.hpp"
#include <algorithm>
#include <sol/sol.hpp>
#include <string>

namespace Engine::LuaAPI {

void registerConnections(sol::state_view lua, Registry& registry) {
    registry.RegisterComponent<SignalConnections>();

    sol::table component = Internal::ensureNamespace(lua, "Component");

    component["connect"] = [&registry](Entity emitter, const std::string& signal, Entity target,
                                       const std::string& method) -> bool {
        if (!registry.IsEntityAlive(emitter) || !registry.IsEntityAlive(target)) {
            return false;
        }
        if (!IsKnownSignal(signal)) {
            log_user_error("[LuaAPI] Component.connect: unknown signal '" + signal + "'");
            return false;
        }
        if (method.empty()) {
            return false;
        }
        if (!registry.HasComponent<SignalConnections>(emitter)) {
            registry.AddComponent<SignalConnections>(emitter, SignalConnections{});
        }
        auto& connections = registry.GetComponent<SignalConnections>(emitter).connections;
        for (const auto& c : connections) {
            if (c.signal == signal && c.target == target && c.method == method) {
                return true;
            }
        }
        connections.push_back(SignalConnection{signal, target, method});
        return true;
    };

    component["disconnect"] = [&registry](Entity emitter, const std::string& signal,
                                          sol::optional<Entity> targetOpt,
                                          sol::optional<std::string> methodOpt) -> bool {
        if (!registry.HasComponent<SignalConnections>(emitter)) {
            return false;
        }
        auto& connections = registry.GetComponent<SignalConnections>(emitter).connections;
        const size_t before = connections.size();
        connections.erase(
            std::remove_if(connections.begin(), connections.end(),
                           [&](const SignalConnection& c) {
                               if (c.signal != signal) {
                                   return false;
                               }
                               if (targetOpt.has_value() && c.target != targetOpt.value()) {
                                   return false;
                               }
                               if (methodOpt.has_value() && c.method != methodOpt.value()) {
                                   return false;
                               }
                               return true;
                           }),
            connections.end());
        return connections.size() != before;
    };

    component["getConnections"] = [&registry](sol::this_state ts, Entity entity) -> sol::table {
        sol::state_view lua(ts);
        sol::table out = lua.create_table();
        if (!registry.HasComponent<SignalConnections>(entity)) {
            return out;
        }
        const auto& connections = registry.GetComponent<SignalConnections>(entity).connections;
        int i = 1;
        for (const auto& c : connections) {
            sol::table t = lua.create_table();
            t["signal"] = c.signal;
            t["target"] = c.target;
            t["method"] = c.method;
            out[i++] = t;
        }
        return out;
    };
}

} // namespace Engine::LuaAPI
