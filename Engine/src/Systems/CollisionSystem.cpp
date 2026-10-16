#include "Systems/CollisionSystem.hpp"
#include "Components/ColliderComponent.hpp"
#include "Components/CollisionEvents.hpp"
#include "Components/Name.hpp"
#include "Components/Position.hpp"
#include "Components/ShapeComponent.hpp"
#include "Scripting/SignalDispatch.hpp"
#include <raylib-cpp.hpp>
#include <raymath.hpp>
#include <vector>

namespace Engine {

void CollisionSystem::Update(Registry& registry, float) {
    std::vector<Entity> collidableEntities;
    GetCollidableEntities(registry, collidableEntities);

    std::set<EntityPair> currentContacts;

    for (size_t i = 0; i < collidableEntities.size(); ++i) {
        Entity const a = collidableEntities[i];
        if (!IsCollidable(registry, a)) {
            continue;
        }

        const auto& colA = registry.GetComponent<ColliderComponent>(a);
        auto& posA = registry.GetComponent<Position>(a);
        static const Rotation defaultRot{0.0f};
        const auto& rotA =
            registry.HasComponent<Rotation>(a) ? registry.GetComponent<Rotation>(a) : defaultRot;
        std::optional<ShapeComponent> shapeA = colA.shape;
        if (!shapeA.has_value() && registry.HasComponent<ShapeComponent>(a)) {
            shapeA = registry.GetComponent<ShapeComponent>(a);
        }
        if (!shapeA.has_value()) {
            continue;
        }
        const auto& vecA = shapeA->GetShapeVectors(rotA, posA);

        for (size_t j = i + 1; j < collidableEntities.size(); ++j) {
            Entity const b = collidableEntities[j];
            if (!IsCollidable(registry, b)) {
                continue;
            }
            const auto& colB = registry.GetComponent<ColliderComponent>(b);
            auto& posB = registry.GetComponent<Position>(b);
            const auto& rotB = registry.HasComponent<Rotation>(b)
                                   ? registry.GetComponent<Rotation>(b)
                                   : defaultRot;
            std::optional<ShapeComponent> shapeB = colB.shape;
            if (!shapeB.has_value() && registry.HasComponent<ShapeComponent>(b)) {
                shapeB = registry.GetComponent<ShapeComponent>(b);
            }
            if (!shapeB.has_value()) {
                continue;
            }
            const auto& vecB = shapeB->GetShapeVectors(rotB, posB);

            if (CheckPolygonCollision(vecA, vecB)) {
                EntityPair pair = MakeOrderedPair(a, b);
                currentContacts.insert(pair);

                const bool isTrigger = colA.isTrigger || colB.isTrigger;

                // lines are skipped
                if (!isTrigger && shapeA.has_value() && shapeB.has_value() &&
                    shapeA->type != ShapeType::LINE && shapeB->type != ShapeType::LINE &&
                    vecA.size() >= 3 && vecB.size() >= 3) {
                    const std::vector<raylib::Vector2> polys[2] = {vecA, vecB};
                    float minOverlap = std::numeric_limits<float>::max();
                    Vector2 mtvAxis = {1.0f, 0.0f};
                    bool overlapping = true;
                    for (auto& poly : polys) {
                        for (size_t i = 0; i < poly.size(); ++i) {
                            const Vector2 va = poly[i];
                            const Vector2 vb = poly[(i + 1) % poly.size()];
                            auto [x, y] = Vector2Subtract(vb, va);
                            const Vector2 perpendicular = {-y, x};
                            const float length = Vector2Length(perpendicular);
                            if (length == 0.0f) {
                                continue;
                            }
                            const Vector2 axis = {perpendicular.x / length,
                                                  perpendicular.y / length};

                            auto [minA, maxA] = ProjectOnAxis(vecA, axis);
                            auto [minB, maxB] = ProjectOnAxis(vecB, axis);
                            const float overlap = std::min(maxA, maxB) - std::max(minA, minB);
                            if (overlap < 0.0f) {
                                overlapping = false;
                                break;
                            }
                            if (overlap < minOverlap) {
                                minOverlap = overlap;
                                const float centerProjA = (minA + maxA) * 0.5f;
                                const float centerProjB = (minB + maxB) * 0.5f;
                                mtvAxis = (centerProjA < centerProjB) ? axis : Vector2Negate(axis);
                            }
                        }
                        if (!overlapping) {
                            break;
                        }
                    }
                    if (overlapping && minOverlap < std::numeric_limits<float>::max()) {
                        // Static colliders don't move
                        const bool aStatic = colA.isStatic;
                        const bool bStatic = colB.isStatic;
                        if (aStatic && bStatic) {
                            // Two statics pushing, then skip resolution
                        } else {
                            const float aFactor = aStatic ? 0.0f : (bStatic ? 1.0f : 0.5f);
                            const float bFactor = bStatic ? 0.0f : (aStatic ? 1.0f : 0.5f);
                            posA.x -= mtvAxis.x * minOverlap * aFactor;
                            posA.y -= mtvAxis.y * minOverlap * aFactor;
                            posB.x += mtvAxis.x * minOverlap * bFactor;
                            posB.y += mtvAxis.y * minOverlap * bFactor;
                        }
                    }
                }

                if (!m_ActiveContacts.contains(pair)) {
                    EnterContact(registry, a, b, isTrigger);
                } else {
                    StayContact(registry, a, b, isTrigger);
                }
            }
        }
    }

    // Fire exit events for contacts that no longer exist
    for (const auto& pair : m_ActiveContacts) {
        if (!currentContacts.contains(pair)) {
            bool isTrigger = false;
            if (registry.HasComponent<ColliderComponent>(pair.first)) {
                isTrigger =
                    isTrigger || registry.GetComponent<ColliderComponent>(pair.first).isTrigger;
            }
            if (registry.HasComponent<ColliderComponent>(pair.second)) {
                isTrigger =
                    isTrigger || registry.GetComponent<ColliderComponent>(pair.second).isTrigger;
            }
            ExitContact(registry, pair.first, pair.second, isTrigger);
        }
    }

    m_ActiveContacts = std::move(currentContacts);
}

CollisionSystem::EntityPair CollisionSystem::MakeOrderedPair(Entity a, Entity b) noexcept {
    if (a < b) {
        return EntityPair{a, b};
    }
    return EntityPair{b, a};
}

void CollisionSystem::GetCollidableEntities(Registry& registry,
                                            std::vector<Entity>& collidableEntities) const {
    collidableEntities.clear();

    if (!m_Entities.empty()) {
        for (Entity const entity : m_Entities) {
            if (IsCollidable(registry, entity)) {
                collidableEntities.push_back(entity);
            }
        }
        return;
    }

    try {
        auto colliders = registry.GetComponentArray<ColliderComponent>();
        collidableEntities.reserve(colliders->size());
        for (const auto& [entity, collider] : *colliders) {
            if (collider.enabled && registry.HasComponent<Position>(entity) &&
                (collider.shape.has_value() || registry.HasComponent<ShapeComponent>(entity))) {
                collidableEntities.push_back(entity);
            }
        }
    } catch (const ErrorEcs&) {
        // Colliders not registered
    }
}

bool CollisionSystem::IsCollidable(Registry& registry, Entity entity) {
    if (!registry.HasComponent<Position>(entity) ||
        !registry.HasComponent<ColliderComponent>(entity)) {
        return false;
    }
    const auto& col = registry.GetComponent<ColliderComponent>(entity);
    return col.enabled && (col.shape.has_value() || registry.HasComponent<ShapeComponent>(entity));
}

void CollisionSystem::EnterContact(Registry& registry, Entity a, Entity b, bool isTrigger) {
    if (registry.HasComponent<CollisionEvents>(a)) {
        auto events = registry.GetComponent<CollisionEvents>(a);
        for (const auto& hook : events.onCollisionEnter) {
            if (hook) {
                hook(a, b);
            }
        }
        if (isTrigger) {
            for (const auto& hook : events.onTriggerEnter) {
                if (hook) {
                    hook(a, b);
                }
            }
        }
    }
    Signals::EmitSignal(registry, a, "onCollisionEnter", b);
    if (isTrigger) {
        Signals::EmitSignal(registry, a, "onTriggerEnter", b);
    }

    if (registry.HasComponent<CollisionEvents>(b)) {
        auto events = registry.GetComponent<CollisionEvents>(b);
        for (const auto& hook : events.onCollisionEnter) {
            if (hook) {
                hook(b, a);
            }
        }
        if (isTrigger) {
            for (const auto& hook : events.onTriggerEnter) {
                if (hook) {
                    hook(b, a);
                }
            }
        }
    }
    Signals::EmitSignal(registry, b, "onCollisionEnter", a);
    if (isTrigger) {
        Signals::EmitSignal(registry, b, "onTriggerEnter", a);
    }
}

void CollisionSystem::StayContact(Registry& registry, const Entity a, const Entity b,
                                  const bool isTrigger) {
    if (registry.HasComponent<CollisionEvents>(a)) {
        auto events = registry.GetComponent<CollisionEvents>(a);
        for (const auto& hook : events.onCollisionStay) {
            if (hook) {
                hook(a, b);
            }
        }
        if (isTrigger) {
            for (const auto& hook : events.onTriggerStay) {
                if (hook) {
                    hook(a, b);
                }
            }
        }
    }
    Signals::EmitSignal(registry, a, "onCollisionStay", b);
    if (isTrigger) {
        Signals::EmitSignal(registry, a, "onTriggerStay", b);
    }

    if (registry.HasComponent<CollisionEvents>(b)) {
        auto events = registry.GetComponent<CollisionEvents>(b);
        for (const auto& hook : events.onCollisionStay) {
            if (hook) {
                hook(b, a);
            }
        }
        if (isTrigger) {
            for (const auto& hook : events.onTriggerStay) {
                if (hook) {
                    hook(b, a);
                }
            }
        }
    }
    Signals::EmitSignal(registry, b, "onCollisionStay", a);
    if (isTrigger) {
        Signals::EmitSignal(registry, b, "onTriggerStay", a);
    }
}

void CollisionSystem::ExitContact(Registry& registry, const Entity a, const Entity b,
                                  const bool isTrigger) {
    if (registry.HasComponent<CollisionEvents>(a)) {
        auto events = registry.GetComponent<CollisionEvents>(a);
        for (const auto& hook : events.onCollisionExit) {
            if (hook) {
                hook(a, b);
            }
        }
        if (isTrigger) {
            for (const auto& hook : events.onTriggerExit) {
                if (hook) {
                    hook(a, b);
                }
            }
        }
    }
    Signals::EmitSignal(registry, a, "onCollisionExit", b);
    if (isTrigger) {
        Signals::EmitSignal(registry, a, "onTriggerExit", b);
    }

    if (registry.HasComponent<CollisionEvents>(b)) {
        auto events = registry.GetComponent<CollisionEvents>(b);
        for (const auto& hook : events.onCollisionExit) {
            if (hook) {
                hook(b, a);
            }
        }
        if (isTrigger) {
            for (const auto& hook : events.onTriggerExit) {
                if (hook) {
                    hook(b, a);
                }
            }
        }
    }
    Signals::EmitSignal(registry, b, "onCollisionExit", a);
    if (isTrigger) {
        Signals::EmitSignal(registry, b, "onTriggerExit", a);
    }
}

std::pair<float, float> CollisionSystem::ProjectOnAxis(const std::vector<raylib::Vector2>& vertices,
                                                       const Vector2 axis) {
    {
        float min = std::numeric_limits<float>::max();
        float max = std::numeric_limits<float>::lowest();

        for (const auto& [x, y] : vertices) {
            const float projection = x * axis.x + y * axis.y;
            if (projection < min) {
                min = projection;
            }
            if (projection > max) {
                max = projection;
            }
        }

        return {min, max};
    }

    std::set<EntityPair> m_ActiveContacts;
};

bool Engine::CollisionSystem::CheckPolygonCollision(const std::vector<raylib::Vector2>& polyA,
                                                    const std::vector<raylib::Vector2>& polyB) {
    const std::vector<raylib::Vector2> vectors[2] = {polyA, polyB};

    for (auto& vec : vectors) {
        const auto currentPoly = vec;
        for (size_t i = 0; i < currentPoly.size(); ++i) {
            const Vector2 va = currentPoly[i];
            const Vector2 vb = currentPoly[(i + 1) % currentPoly.size()];

            auto [x, y] = Vector2Subtract(vb, va);

            const Vector2 perpendicular = {-y, x};
            const float length = Vector2Length(perpendicular);
            if (length == 0.0f) {
                continue;
            }
            const Vector2 axis = {perpendicular.x / length, perpendicular.y / length};

            auto [minA, maxA] = ProjectOnAxis(polyA, axis);
            auto [minB, maxB] = ProjectOnAxis(polyB, axis);

            if (maxA < minB || maxB < minA) {
                return false;
            }
        }
    }

    return true;
}

} // namespace Engine
