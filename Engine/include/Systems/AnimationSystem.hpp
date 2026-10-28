#ifndef ENGINE_SYSTEMS_ANIMATIONSYSTEM_HPP_
#define ENGINE_SYSTEMS_ANIMATIONSYSTEM_HPP_

#include "ECS/LayeredRenderSystem.hpp"

namespace Engine {
class Registry;

class AnimationSystem : public LayeredRenderSystem {
  public:
    void Update(Registry& registry, float dt) override;

    bool CanDraw(Registry& registry, Entity entity, bool screenSpace) override;

    void DrawEntity(Registry& registry, Entity entity) override;
};
} // namespace Engine

#endif // ENGINE_SYSTEMS_ANIMATIONSYSTEM_HPP_
