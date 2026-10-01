#ifndef ENGINE_PLUGINS_TRANSFORMPLUGIN_HPP_
#define ENGINE_PLUGINS_TRANSFORMPLUGIN_HPP_
#include "Components/Position.hpp"
#include "Plugins/IPlugin.hpp"
#include "Systems/TransformSystem.hpp"

namespace Engine {

/**
 * @class TransformPlugin
 * @brief An example plugin that bundles the Position component and TransformSystem.
 *
 * Users of the engine can just call `transformPlugin.Init(registry)` to
 * integrate position features into their game.
 */
class TransformPlugin : public IPlugin {
  public:
    /**
     * @brief Initializes the Transform plugin.
     * @param registry The main ECS registry.
     */
    void Init(Registry& registry) override {
        registry.RegisterComponent<Position>();

        registry.RegisterSystem<TransformSystem>();
    }
};
} // namespace Engine

#endif // ENGINE_PLUGINS_TRANSFORMPLUGIN_HPP_
