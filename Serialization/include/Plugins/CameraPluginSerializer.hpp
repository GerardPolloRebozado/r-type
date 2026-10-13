#ifndef SERIALIZATION_PLUGINS_CAMERAPLUGINSERIALIZER_HPP_
#define SERIALIZATION_PLUGINS_CAMERAPLUGINSERIALIZER_HPP_

#include "Plugins/IPluginSerializer.hpp"

namespace Serialization {

class CameraPluginSerializer : public IPluginSerializer {
  public:
    const char* GetKey() const override {
        return "CameraComponent";
    }

    void Serialize(Engine::Entity entity, Engine::Registry& registry,
                   nlohmann::json& outEntityJson) override;
    void Deserialize(Engine::Entity entity, Engine::Registry& registry,
                     const nlohmann::json& inEntityJson) override;
};

} // namespace Serialization

#endif // SERIALIZATION_PLUGINS_CAMERAPLUGINSERIALIZER_HPP_
