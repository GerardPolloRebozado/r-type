#ifndef ENGINE_PLUGINS_PLUGINEXAMPLE_HPP_
#define ENGINE_PLUGINS_PLUGINEXAMPLE_HPP_

#include "Components/ComponentExample.hpp"
#include "Components/EventsExample.hpp"
#include "Plugins/IPlugin.hpp"
#include "Systems/SystemExample.hpp"

namespace Engine {
/**
 * @brief A template plugin demonstrating how to bundle Data + Logic + Events.
 */
class PluginExample : public IPlugin {
  public:
    void Init(Registry& registry) override {
        registry.RegisterComponent<ComponentExample>();
        registry.RegisterComponent<EventsExample>();
        registry.RegisterSystem<SystemExample>();
    }
};
} // namespace Engine

#endif // ENGINE_PLUGINS_PLUGINEXAMPLE_HPP_
