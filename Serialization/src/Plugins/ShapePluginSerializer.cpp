#include "Plugins/ShapePluginSerializer.hpp"
#include "Plugins/ShapePlugin.hpp"

namespace Serialization {

nlohmann::json ShapePluginSerializer::SerializeShapeData(const Engine::ShapeComponent& shape) {
    return {
        {"type", static_cast<int>(shape.type)},
        {"width", shape.width},
        {"height", shape.height},
        {"radius", shape.radius},
        {"sides", shape.sides},
        {"endOffsetX", shape.endOffset.x},
        {"endOffsetY", shape.endOffset.y},
        {"thickness", shape.thickness},
        {"filled", shape.filled},
        {"color_r", shape.color.r},
        {"color_g", shape.color.g},
        {"color_b", shape.color.b},
        {"color_a", shape.color.a},
    };
}

Engine::ShapeComponent ShapePluginSerializer::DeserializeShapeData(const nlohmann::json& json) {
    Engine::ShapeComponent shape;
    shape.type = static_cast<Engine::ShapeType>(json.value("type", 0));
    shape.width = json.value("width", 50.0f);
    shape.height = json.value("height", 50.0f);
    shape.radius = json.value("radius", 25.0f);
    shape.sides = json.value("sides", 3);
    shape.endOffset.x = json.value("endOffsetX", 50.0f);
    shape.endOffset.y = json.value("endOffsetY", 50.0f);
    shape.thickness = json.value("thickness", 1.0f);
    shape.filled = json.value("filled", true);
    shape.color.r = static_cast<unsigned char>(json.value("color_r", 255));
    shape.color.g = static_cast<unsigned char>(json.value("color_g", 255));
    shape.color.b = static_cast<unsigned char>(json.value("color_b", 255));
    shape.color.a = static_cast<unsigned char>(json.value("color_a", 255));
    return shape;
}

void ShapePluginSerializer::Serialize(Engine::Entity entity, Engine::Registry& registry,
                                      nlohmann::json& outEntityJson) {
    if (registry.HasComponent<Engine::ShapeComponent>(entity)) {
        const auto& shape = registry.GetComponent<Engine::ShapeComponent>(entity);
        outEntityJson["ShapeComponent"] = SerializeShapeData(shape);
    }
}

void ShapePluginSerializer::Deserialize(Engine::Entity entity, Engine::Registry& registry,
                                        const nlohmann::json& inEntityJson) {
    if (inEntityJson.contains("ShapeComponent")) {
        Engine::ShapeComponent shape = DeserializeShapeData(inEntityJson["ShapeComponent"]);
        Engine::ShapePlugin::AttachToEntity(registry, entity, shape);
    }
}

} // namespace Serialization
