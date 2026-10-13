#ifndef SERIALIZATION_PLUGINS_SCRIPTPLUGINSERIALIZER_HPP_
#define SERIALIZATION_PLUGINS_SCRIPTPLUGINSERIALIZER_HPP_

#include "Plugins/IPluginSerializer.hpp"

namespace Serialization {

class ScriptPluginSerializer : public IPluginSerializer {
  public:
    const char* GetKey() const override {
        return "ScriptComponent";
    }

    void Serialize(Engine::Entity entity, Engine::Registry& registry,
                   nlohmann::json& outEntityJson) override;
    void Deserialize(Engine::Entity entity, Engine::Registry& registry,
                     const nlohmann::json& inEntityJson) override;
};

} // namespace Serialization

#endif // SERIALIZATION_PLUGINS_SCRIPTPLUGINSERIALIZER_HPP_