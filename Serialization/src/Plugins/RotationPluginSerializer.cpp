#include "Plugins/RotationPluginSerializer.hpp"
#include "Components/Rotation.hpp"
#include "Plugins/RotationPlugin.hpp"

namespace Serialization {

void RotationPluginSerializer::Serialize(Engine::Entity entity, Engine::Registry& registry,
                                         nlohmann::json& outEntityJson) {
    if (registry.HasComponent<Engine::Rotation>(entity)) {
        const auto& rot = registry.GetComponent<Engine::Rotation>(entity);
        outEntityJson["RotationComponent"] = {{"angle", rot.angle}};
    }
}

void RotationPluginSerializer::Deserialize(Engine::Entity entity, Engine::Registry& registry,
                                           const nlohmann::json& inEntityJson) {
    if (inEntityJson.contains("RotationComponent")) {
        const auto& json = inEntityJson["RotationComponent"];
        float angle = json.value("angle", 0.0f);
        Engine::RotationPlugin::AttachToEntity(registry, entity, angle);
    }
}

} // namespace Serialization
