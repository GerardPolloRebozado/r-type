#include "Systems/CollisionSystem.hpp"
#include "Components/Collider.hpp"
#include "Components/CollisionEvents.hpp"
#include "Components/Position.hpp"
#include <vector>

namespace Engine {
namespace helpers {
struct Aabb {
    float minX;
    float minY;
    float maxX;
    float maxY;
};

Aabb BuildAabb(Registry& registry, Entity entity) {
    Position& position = registry.GetComponent<Position>(entity);
    Collider& collider = registry.GetComponent<Collider>(entity);
    float centerX = position.x + collider.offsetX;
    float centerY = position.y + collider.offsetY;
    float halfW = collider.width * 0.5f;
    float halfH = collider.height * 0.5f;

    Aabb box;
    box.minX = centerX - halfW;
    box.maxX = centerX + halfW;
    box.minY = centerY - halfH;
    box.maxY = centerY + halfH;
    return box;
}

bool AabbOverlap(const Aabb& a, const Aabb& b) {
    return a.minX < b.maxX && a.maxX > b.minX && a.minY < b.maxY && a.maxY > b.minY;
}
} // namespace helpers

void CollisionSystem::Update(Registry& registry, float dt) {
    (void)dt;

    std::vector<Entity> movedEntities;
    std::vector<Entity> collidableEntities;

    // Pass 1: who can collide, and who moved this frame
    GetCollidableEntities(registry, collidableEntities, movedEntities);

    // Pass 2: each moved entity vs every collider. Update contact set + game callbacks.
    for (Entity const moved : movedEntities) {
        for (Entity const other : collidableEntities) {
            if (moved == other) {
                continue;
            }

            EntityPair pair = MakeOrderedPair(moved, other);
            bool overlapping = AreOverlapping(registry, moved, other);

            if (overlapping) {
                // First frame of contact: fire enter once, then remember the pair.
                if (m_ActiveContacts.find(pair) == m_ActiveContacts.end()) {
                    EnterContact(registry, moved, other);
                    m_ActiveContacts.insert(pair);
                }
            } else if (m_ActiveContacts.find(pair) != m_ActiveContacts.end()) {
                // Was touching last time we tracked it, but not anymore, so fire exit.
                ExitContact(registry, moved, other);
                m_ActiveContacts.erase(pair);
            }
        }
    }
}

CollisionSystem::EntityPair CollisionSystem::MakeOrderedPair(Entity a, Entity b) {
    // Same pair regardless of argument order (needed for std::set lookup).
    if (a < b) {
        return EntityPair{a, b};
    }
    return EntityPair{b, a};
}

void CollisionSystem::GetCollidableEntities(Registry& registry,
                                            std::vector<Entity>& collidableEntities,
                                            std::vector<Entity>& movedEntities) const {
    collidableEntities.clear();
    movedEntities.clear();

    for (Entity const entity : m_Entities) {
        if (!IsCollidable(registry, entity)) {
            continue;
        }
        collidableEntities.push_back(entity);
        if (registry.GetComponent<Position>(entity).moved) {
            movedEntities.push_back(entity);
        }
    }
}

bool CollisionSystem::IsCollidable(Registry& registry, Entity entity) {
    if (!registry.HasComponent<Position>(entity) || !registry.HasComponent<Collider>(entity)) {
        return false;
    }
    return registry.GetComponent<Collider>(entity).enabled;
}

bool CollisionSystem::AreOverlapping(Registry& registry, Entity a, Entity b) {
    if (!IsCollidable(registry, a) || !IsCollidable(registry, b)) {
        return false;
    }
    helpers::Aabb boxA = helpers::BuildAabb(registry, a);
    helpers::Aabb boxB = helpers::BuildAabb(registry, b);
    return helpers::AabbOverlap(boxA, boxB);
}

void CollisionSystem::EnterContact(Registry& registry, Entity a, Entity b) {
    // Notify both sides; each callback gets (self, other) with the correct "self" id.
    if (registry.HasComponent<CollisionEvents>(a)) {
        CollisionEvents& events = registry.GetComponent<CollisionEvents>(a);
        for (auto& hook : events.onCollisionEnter) {
            hook(a, b);
        }
    }
    if (registry.HasComponent<CollisionEvents>(b)) {
        CollisionEvents& events = registry.GetComponent<CollisionEvents>(b);
        for (auto& hook : events.onCollisionEnter) {
            hook(b, a);
        }
    }
}

void CollisionSystem::ExitContact(Registry& registry, Entity a, Entity b) {
    if (registry.HasComponent<CollisionEvents>(a)) {
        CollisionEvents& events = registry.GetComponent<CollisionEvents>(a);
        for (auto& hook : events.onCollisionExit) {
            hook(a, b);
        }
    }
    if (registry.HasComponent<CollisionEvents>(b)) {
        CollisionEvents& events = registry.GetComponent<CollisionEvents>(b);
        for (auto& hook : events.onCollisionExit) {
            hook(b, a);
        }
    }
}
} // namespace Engine
