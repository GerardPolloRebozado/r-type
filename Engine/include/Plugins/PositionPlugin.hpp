#ifndef ENGINE_PLUGINS_POSITIONPLUGIN_HPP_
#define ENGINE_PLUGINS_POSITIONPLUGIN_HPP_
#include "Components/Position.hpp"
#include "Plugins/IPlugin.hpp"
#include "Systems/PositionSystem.hpp"

namespace Engine {

/**
 * @class PositionPlugin
 * @brief An example plugin that bundles the Position component and PositionSystem.
 *
 * Users of the engine can just call `positionPlugin.Init(registry)` to
 * integrate position features into their game.
 */
class PositionPlugin : public IPlugin {
  public:
    /**
     * @brief Initializes the Position plugin.
     * @param registry The main ECS registry.
     */
    void Init(Registry& registry) override;
};
} // namespace Engine

#endif // ENGINE_PLUGINS_POSITIONPLUGIN_HPP_
