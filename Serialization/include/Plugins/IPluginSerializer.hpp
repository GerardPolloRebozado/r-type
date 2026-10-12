#ifndef SERIALIZATION_PLUGINS_IPLUGINSERIALIZER_HPP_
#define SERIALIZATION_PLUGINS_IPLUGINSERIALIZER_HPP_

#include "ECS/Registry.hpp"
#include <nlohmann/json.hpp>

namespace Serialization {

/**
 * @class IPluginSerializer
 * @brief Interface for modular component serialization.
 *
 * Each component type implements this interface in its own dedicated file,
 * keeping serialization decoupled and modular.
 */
class IPluginSerializer {
  public:
    virtual ~IPluginSerializer() = default;

    virtual const char* GetKey() const = 0;
    virtual void Serialize(Engine::Entity entity, Engine::Registry& registry,
                           nlohmann::json& outEntityJson) = 0;
    virtual void Deserialize(Engine::Entity entity, Engine::Registry& registry,
                             const nlohmann::json& inEntityJson) = 0;
};

} // namespace Serialization

#endif // SERIALIZATION_PLUGINS_IPLUGINSERIALIZER_HPP_
