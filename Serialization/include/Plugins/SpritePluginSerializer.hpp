#ifndef SERIALIZATION_PLUGINS_SPRITEPLUGINSERIALIZER_HPP_
#define SERIALIZATION_PLUGINS_SPRITEPLUGINSERIALIZER_HPP_

#include "Plugins/IPluginSerializer.hpp"

namespace Serialization {

class SpritePluginSerializer : public IPluginSerializer {
  public:
    const char* GetKey() const override {
        return "SpriteComponent";
    }

    void Serialize(Engine::Entity entity, Engine::Registry& registry,
                   nlohmann::json& outEntityJson) override;
    void Deserialize(Engine::Entity entity, Engine::Registry& registry,
                     const nlohmann::json& inEntityJson) override;
};

} // namespace Serialization

#endif // SERIALIZATION_PLUGINS_SPRITEPLUGINSERIALIZER_HPP_
