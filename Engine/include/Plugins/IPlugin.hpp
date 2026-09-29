#ifndef ENGINE_PLUGINS_IPLUGIN_HPP_
#define ENGINE_PLUGINS_IPLUGIN_HPP_
#include "ECS/Registry.hpp"

namespace Engine {
/**
 * @class IPlugin
 * @brief Interface for plugins.
 *
 * Plugins allow users to inject combinations of components, systems,
 * and functionality into the engine.
 */
class IPlugin {
  public:
    virtual ~IPlugin() = default;

    /**
     * @brief Initializes the plugin, registering components and systems.
     * @param registry A reference to the main ECS registry.
     */
    virtual void Init(Registry& registry) = 0;
};
} // namespace Engine

#endif // ENGINE_PLUGINS_IPLUGIN_HPP_
