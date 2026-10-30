#include "Utils/EntityFlip.hpp"
#include "Components/ColliderComponent.hpp"
#include "Components/Position.hpp"
#include "Components/ShapeComponent.hpp"
#include "Components/SpriteComponent.hpp"
#include "ECS/Registry.hpp"
#include "Systems/HierarchySystem.hpp"
#include <cmath>

namespace Engine {

namespace {

/**
 * @brief Resolves which shape describes `entity`'s footprint, the way
 *        CollisionSystem does: the collider's own shape when it has one, else
 *        the entity's ShapeComponent.
 * @return nullptr when neither exists.
 */
const ShapeComponent* FootprintShape(Registry& registry, Entity entity) {
    if (registry.HasComponent<ColliderComponent>(entity)) {
        const auto& collider = registry.RequireComponent<ColliderComponent>(entity);
        if (collider.shape.has_value() && collider.shape->width > 0.0f) {
            return &collider.shape.value();
        }
    }
    if (registry.IsComponentRegistered<ShapeComponent>() &&
        registry.HasComponent<ShapeComponent>(entity)) {
        const auto& shape = registry.RequireComponent<ShapeComponent>(entity);
        if (shape.width > 0.0f) {
            return &shape;
        }
    }
    return nullptr;
}

/**
 * @brief Width of `entity`'s footprint, in world units: its shape when it has
 *        one (that is what everything else collides with), else the drawn
 *        sprite, else 0 for a marker entity with neither.
 */
float FootprintWidth(Registry& registry, Entity entity) {
    if (const ShapeComponent* shape = FootprintShape(registry, entity)) {
        return shape->width;
    }
    if (registry.HasComponent<SpriteComponent>(entity)) {
        const auto& sprite = registry.RequireComponent<SpriteComponent>(entity);
        if (sprite.sprite) {
            return std::fabs(sprite.sprite->GetSourceRect().width) * sprite.scaleX;
        }
    }
    return 0.0f;
}

/**
 * @brief World x of the left edge of `entity`'s footprint: shapes start at the
 *        entity position (see ShapeComponent::GetShapeVectors), sprites are
 *        drawn at the entity position plus their offset.
 */
float FootprintLeft(Registry& registry, Entity entity) {
    const auto& position = registry.RequireComponent<Position>(entity);
    if (FootprintShape(registry, entity) != nullptr) {
        return position.x;
    }
    if (registry.HasComponent<SpriteComponent>(entity)) {
        const auto& sprite = registry.RequireComponent<SpriteComponent>(entity);
        if (sprite.sprite) {
            return position.x + sprite.offsetX;
        }
    }
    return position.x;
}

/**
 * @brief Reflects `node`'s subtree across the vertical line x = @p axis.
 *
 * The whole subtree is reflected about the same line - the rig root's centre -
 * so a grandchild keeps its distance from its parent.
 */
void MirrorSubtree(Registry& registry, Entity node, float axis) {
    for (Entity child : HierarchySystem::FindDirectChildren(registry, node)) {
        if (!registry.HasComponent<Position>(child)) {
            continue;
        }
        const float width = FootprintWidth(registry, child);
        const float centre = FootprintLeft(registry, child) + width / 2.0f;

        Position& position = registry.RequireComponent<Position>(child);
        position.x = (2.0f * axis - centre) - width / 2.0f;

        MirrorSubtree(registry, child, axis);
    }
}

} // namespace

bool FlipEntityX(Registry& registry, Entity entity) {
    if (!registry.IsEntityAlive(entity) || !registry.HasComponent<Position>(entity)) {
        return false;
    }

    if (registry.HasComponent<SpriteComponent>(entity)) {
        SpriteComponent& sprite = registry.RequireComponent<SpriteComponent>(entity);
        sprite.flipX = !sprite.flipX;
    }

    const float centre = FootprintLeft(registry, entity) + FootprintWidth(registry, entity) / 2.0f;
    MirrorSubtree(registry, entity, centre);
    return true;
}

} // namespace Engine
