#ifndef ENGINE_PLUGINS_SPRITEPLUGIN_HPP_
#define ENGINE_PLUGINS_SPRITEPLUGIN_HPP_

#include "Components/Position.hpp"
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
     * Also registers Position, which SpriteSystem needs to place sprites.
     * @param registry The ECS registry where components and systems are registered.
     */
    void Init(Registry& registry) override {
        registry.RegisterComponent<Position>();
        registry.RegisterComponent<SpriteComponent>();
        registry.RegisterSystem<SpriteSystem>();
    }

    /**
     * @brief Attaches a SpriteComponent to an entity and ensures it has a Position.
     * An existing Position is left untouched.
     * @param registry The ECS registry containing the entity.
     * @param entity The entity to attach the sprite to.
     * @param sprite The sprite component to attach.
     */
    static void AttachToEntity(Registry& registry, Entity entity, const SpriteComponent& sprite) {
        registry.AddComponent(entity, sprite);
        registry.AddComponent(entity, Position{});
    }
};
} // namespace Engine

#endif // ENGINE_PLUGINS_SPRITEPLUGIN_HPP_
