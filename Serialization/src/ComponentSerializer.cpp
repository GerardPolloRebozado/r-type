#include "ComponentSerializer.hpp"
#include "Plugins/CameraPluginSerializer.hpp"
#include "Plugins/ColliderPluginSerializer.hpp"
#include "Plugins/PositionPluginSerializer.hpp"
#include "Plugins/RotationPluginSerializer.hpp"
#include "Plugins/ScriptPluginSerializer.hpp"
#include "Plugins/ShapePluginSerializer.hpp"
#include "Plugins/SpritePluginSerializer.hpp"
#include "Plugins/TextPluginSerializer.hpp"

namespace Serialization {

std::vector<std::unique_ptr<IPluginSerializer>> ComponentSerializer::s_Serializers;

ComponentSerializer::ComponentSerializer() {
    EnsureInitialized();
}

void ComponentSerializer::EnsureInitialized() {
    if (!s_Serializers.empty()) {
        return;
    }
    s_Serializers.push_back(std::make_unique<PositionPluginSerializer>());
    s_Serializers.push_back(std::make_unique<RotationPluginSerializer>());
    s_Serializers.push_back(std::make_unique<TextPluginSerializer>());
    s_Serializers.push_back(std::make_unique<ShapePluginSerializer>());
    s_Serializers.push_back(std::make_unique<SpritePluginSerializer>());
    s_Serializers.push_back(std::make_unique<ColliderPluginSerializer>());
    s_Serializers.push_back(std::make_unique<CameraPluginSerializer>());
    s_Serializers.push_back(std::make_unique<ScriptPluginSerializer>());
}

void ComponentSerializer::RegisterSerializer(std::unique_ptr<IPluginSerializer> serializer) {
    EnsureInitialized();
    s_Serializers.push_back(std::move(serializer));
}

const std::vector<std::unique_ptr<IPluginSerializer>>& ComponentSerializer::GetSerializers() {
    EnsureInitialized();
    return s_Serializers;
}

void ComponentSerializer::SerializeAll(Engine::Entity entity, Engine::Registry& registry,
                                       nlohmann::json& outEntityJson) {
    EnsureInitialized();
    for (const auto& serializer : s_Serializers) {
        serializer->Serialize(entity, registry, outEntityJson);
    }
}

void ComponentSerializer::DeserializeAll(Engine::Entity entity, Engine::Registry& registry,
                                         const nlohmann::json& inEntityJson) {
    EnsureInitialized();
    for (const auto& serializer : s_Serializers) {
        serializer->Deserialize(entity, registry, inEntityJson);
    }
}

} // namespace Serialization
