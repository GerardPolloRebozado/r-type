#ifndef ENGINE_SYSTEMS_SPRITESYSTEM_HPP_
#define ENGINE_SYSTEMS_SPRITESYSTEM_HPP_

#include "ECS/System.hpp"

namespace Engine {
class Registry;

/**
 * @class SpriteSystem
 * @brief System that updates and draws SpriteComponent based on Transform.
 */
class SpriteSystem : public System {
public:
    void Update(Registry& registry, float dt) override;
    void Render(Registry& registry) override;
};
} // namespace Engine

#endif // ENGINE_SYSTEMS_SPRITESYSTEM_HPP_
