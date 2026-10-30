#include "Plugins/OpacityPlugin.hpp"
#include "Components/Parent.hpp"

namespace Engine {

void OpacityPlugin::Init(Registry& registry) {
    registry.RegisterComponent<Opacity>();
}

void OpacityPlugin::AttachToEntity(Registry& registry, Entity entity, float alpha,
                                   bool inheritToChildren) {
    registry.AddComponent(entity, Opacity::Create(alpha, inheritToChildren));
}

float OpacityPlugin::GetEffectiveAlpha(Registry& registry, Entity entity) {
    float result = 1.0f;
    if (registry.HasComponent<Opacity>(entity)) {
        result = registry.RequireComponent<Opacity>(entity).alpha;
    }
    if (!registry.IsComponentRegistered<Parent>()) {
        return result;
    }
    // Walk up: each ancestor contributes its alpha only when it passes its
    // opacity to children. The flag gates the node's own value, so ancestors
    // above an opted-out node still flow through.
    Entity current = entity;
    for (int depth = 0; depth < 64; ++depth) {
        if (!registry.HasComponent<Parent>(current)) {
            break;
        }
        current = registry.RequireComponent<Parent>(current).parent;
        if (!registry.IsEntityAlive(current) || !registry.HasComponent<Opacity>(current)) {
            if (!registry.IsEntityAlive(current)) {
                break;
            }
            continue;
        }
        const auto& parentOpacity = registry.RequireComponent<Opacity>(current);
        if (parentOpacity.inheritToChildren) {
            result *= parentOpacity.alpha;
        }
    }
    return result;
}

} // namespace Engine
