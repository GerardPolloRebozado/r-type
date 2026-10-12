#ifndef SERIALIZATION_PLUGINS_SHAPEPLUGINSERIALIZER_HPP_
#define SERIALIZATION_PLUGINS_SHAPEPLUGINSERIALIZER_HPP_

#include "Components/ShapeComponent.hpp"
#include "Plugins/IPluginSerializer.hpp"

namespace Serialization {

class ShapePluginSerializer : public IPluginSerializer {
  public:
    const char* GetKey() const override {
        return "ShapeComponent";
    }

    void Serialize(Engine::Entity entity, Engine::Registry& registry,
                   nlohmann::json& outEntityJson) override;
    void Deserialize(Engine::Entity entity, Engine::Registry& registry,
                     const nlohmann::json& inEntityJson) override;

    static nlohmann::json SerializeShapeData(const Engine::ShapeComponent& shape);
    static Engine::ShapeComponent DeserializeShapeData(const nlohmann::json& json);
};

} // namespace Serialization

#endif // SERIALIZATION_PLUGINS_SHAPEPLUGINSERIALIZER_HPP_
