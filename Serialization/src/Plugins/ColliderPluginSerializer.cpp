#include "Plugins/ColliderPluginSerializer.hpp"
#include "Components/ColliderComponent.hpp"
#include "Plugins/ColliderPlugin.hpp"
#include "Plugins/ShapePluginSerializer.hpp"

namespace Serialization {

void ColliderPluginSerializer::Serialize(Engine::Entity entity, Engine::Registry& registry,
                                         nlohmann::json& outEntityJson) {
    if (registry.HasComponent<Engine::ColliderComponent>(entity)) {
        const auto& collider = registry.GetComponent<Engine::ColliderComponent>(entity);
        nlohmann::json colliderJson = {
            {"isTrigger", collider.isTrigger},
            {"enabled", collider.enabled},
        };
        if (collider.shape.has_value()) {
            colliderJson["shape"] = ShapePluginSerializer::SerializeShapeData(*collider.shape);
        }
        outEntityJson["ColliderComponent"] = colliderJson;
    }
}

void ColliderPluginSerializer::Deserialize(Engine::Entity entity, Engine::Registry& registry,
                                           const nlohmann::json& inEntityJson) {
    if (inEntityJson.contains("ColliderComponent")) {
        const auto& json = inEntityJson["ColliderComponent"];
        Engine::ColliderComponent collider;
        collider.isTrigger = json.value("isTrigger", false);
        collider.enabled = json.value("enabled", true);
        if (json.contains("shape")) {
            collider.shape = ShapePluginSerializer::DeserializeShapeData(json["shape"]);
        }
        Engine::ColliderPlugin::AttachToEntity(registry, entity, collider);
    }
}

} // namespace Serialization
