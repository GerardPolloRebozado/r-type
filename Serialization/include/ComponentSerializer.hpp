#ifndef SERIALIZATION_COMPONENTSERIALIZER_HPP_
#define SERIALIZATION_COMPONENTSERIALIZER_HPP_

#include "ECS/Registry.hpp"
#include "Plugins/IPluginSerializer.hpp"
#include <memory>
#include <nlohmann/json.hpp>
#include <vector>

namespace Serialization {

class ComponentSerializer {
  public:
    ComponentSerializer();

    static void SerializeAll(Engine::Entity entity, Engine::Registry& registry,
                             nlohmann::json& outEntityJson);
    static void DeserializeAll(Engine::Entity entity, Engine::Registry& registry,
                               const nlohmann::json& inEntityJson);

    static void RegisterSerializer(std::unique_ptr<IPluginSerializer> serializer);
    static const std::vector<std::unique_ptr<IPluginSerializer>>& GetSerializers();

  private:
    static void EnsureInitialized();
    static std::vector<std::unique_ptr<IPluginSerializer>> s_Serializers;
};

} // namespace Serialization

#endif // SERIALIZATION_COMPONENTSERIALIZER_HPP_
