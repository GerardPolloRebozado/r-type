#include "Plugins/ScriptPluginSerializer.hpp"
#include "ECS/Registry.hpp"
#include "Scripting/ScriptComponent.hpp"

namespace Serialization {

void ScriptPluginSerializer::Serialize(Engine::Entity entity, Engine::Registry& registry,
                                       nlohmann::json& outEntityJson) {
    if (registry.HasComponent<Engine::ScriptComponent>(entity)) {
        const auto& script = registry.GetComponent<Engine::ScriptComponent>(entity);
        outEntityJson[GetKey()] = {
            {"scriptPath", script.scriptPath},
            {"autoReload", script.autoReload},
        };
    }
}

void ScriptPluginSerializer::Deserialize(Engine::Entity entity, Engine::Registry& registry,
                                         const nlohmann::json& inEntityJson) {
    if (inEntityJson.contains(GetKey())) {
        const auto& scriptJson = inEntityJson.at(GetKey());
        Engine::ScriptComponent script;
        script.scriptPath = scriptJson.value("scriptPath", "");
        script.autoReload = scriptJson.value("autoReload", true);
        registry.AddComponent<Engine::ScriptComponent>(entity, script);
    }
}

} // namespace Serialization
