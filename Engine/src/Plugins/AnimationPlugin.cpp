#include "Plugins/AnimationPlugin.hpp"
#include "Assets/AssetManager.hpp"

namespace Engine {

void AnimationPlugin::Init(Registry& registry) {
    if (!registry.HasResource<AssetManager>()) {
        registry.AddResource<AssetManager>();
    }
    registry.RegisterComponent<Position>();
    registry.RegisterComponent<Animation>();
    registry.RegisterSystem<AnimationSystem>();
}

void AnimationPlugin::AttachToEntity(Registry& registry, Entity entity,
                                     const Animation& animation) {
    Animation anim = animation;
    if (anim.currentAnimation.empty() && !anim.clips.empty()) {
        anim.currentAnimation = anim.clips.begin()->first;
    }
    registry.AddComponent(entity, std::move(anim));
    if (!registry.HasComponent<Position>(entity)) {
        registry.AddComponent(entity, Position());
    }
}

} // namespace Engine
