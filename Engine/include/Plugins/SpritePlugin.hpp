#ifndef ENGINE_PLUGINS_SPRITEPLUGIN_HPP_
#define ENGINE_PLUGINS_SPRITEPLUGIN_HPP_

#include "Components/SpriteComponent.hpp"
#include "Plugins/IPlugin.hpp"
#include "Systems/SpriteSystem.hpp"

namespace Engine {

/**
 * @class SpritePlugin
 * @brief Plugin that bundles the SpriteComponent and SpriteSystem.
 */
class SpritePlugin : public IPlugin {
public:
    /**
     * @brief Initializes the SpritePlugin by registering the SpriteComponent and SpriteSystem.
     * @param registry The ECS registry where components and systems are registered.
     */
    void Init(Registry& registry) override {
        registry.RegisterComponent<SpriteComponent>();
        registry.RegisterSystem<SpriteSystem>();
    }
};
} // namespace Engine

#endif // ENGINE_PLUGINS_SPRITEPLUGIN_HPP_
