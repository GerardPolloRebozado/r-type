#include "Plugins/PositionPluginSerializer.hpp"
#include "Components/Position.hpp"

namespace Serialization {

void PositionPluginSerializer::Serialize(Engine::Entity entity, Engine::Registry& registry,
                                         nlohmann::json& outEntityJson) {
    if (registry.HasComponent<Engine::Position>(entity)) {
        const auto& position = registry.GetComponent<Engine::Position>(entity);
        outEntityJson["PositionComponent"] = {
            {"x", position.x}, {"y", position.y}, {"z", position.z}};
    }
}

void PositionPluginSerializer::Deserialize(Engine::Entity entity, Engine::Registry& registry,
                                           const nlohmann::json& inEntityJson) {
    if (inEntityJson.contains("PositionComponent")) {
        const auto& positionJson = inEntityJson["PositionComponent"];
        float x = positionJson.value("x", 0.0f);
        float y = positionJson.value("y", 0.0f);
        int z = positionJson.value("z", 0);
        registry.AddComponent<Engine::Position>(entity, Engine::Position(x, y, z));
    } else if (inEntityJson.contains("TransformComponent")) {
        const auto& positionJson = inEntityJson["TransformComponent"];
        float x = positionJson.value("x", 0.0f);
        float y = positionJson.value("y", 0.0f);
        int z = positionJson.value("z", 0);
        registry.AddComponent<Engine::Position>(entity, Engine::Position(x, y, z));
    }
}

} // namespace Serialization
