#ifndef ENGINE_PLUGINS_PLUGINEXAMPLE_HPP_
#define ENGINE_PLUGINS_PLUGINEXAMPLE_HPP_

#include "Components/ComponentExample.hpp"
#include "Components/PluginExampleEvents.hpp"
#include "Plugins/IPlugin.hpp"
#include "Systems/SystemExample.hpp"

namespace Engine {
/**
 * @brief A template plugin demonstrating how to bundle Data + Logic + Events.
 */
class PluginExample : public IPlugin {
  public:
    void Init(Registry& registry) override;

    /**
     * @brief A helper function to easily bundle all related plugin components onto a specific
     * entity. Prevents the game developer from having to manually call AddComponent multiple times.
     */
    static void AttachToEntity(Registry& registry, Entity entity);
};
} // namespace Engine

#endif // ENGINE_PLUGINS_PLUGINEXAMPLE_HPP_
