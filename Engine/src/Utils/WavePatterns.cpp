#include "Utils/WavePatterns.hpp"
#include "Components/ColliderComponent.hpp"
#include "Components/Parent.hpp"
#include "Components/Position.hpp"
#include "Components/SpriteComponent.hpp"
#include "Resources/PrefabSpawner.hpp"
#include "Scripting/ScriptComponent.hpp"
#include "Systems/HierarchySystem.hpp"
#include <cmath>

namespace Engine {

EntityBounds GetEntitySize(Registry& registry, const Entity entity) {
    EntityBounds bounds{64.f, 64.f, 0.f, 0.f}; // default size
    if (registry.HasComponent<ColliderComponent>(entity)) {
        const auto colliderOpt = registry.GetComponent<ColliderComponent>(entity);
        if (colliderOpt.has_value()) {
            const auto& collider = colliderOpt->get();
            if (collider.shape && collider.shape->type == ShapeType::RECTANGLE) {
                bounds.width = collider.shape->width;
                bounds.height = collider.shape->height;
                bounds.offsetX = 0.f;
                bounds.offsetY = 0.f;
                return bounds;
            }
        }
    }
    if (registry.HasComponent<SpriteComponent>(entity)) {
        const auto spriteOpt = registry.GetComponent<SpriteComponent>(entity);
        if (spriteOpt.has_value()) {
            const auto& spriteComp = spriteOpt->get();
            if (spriteComp.sprite) {
                const auto& src = spriteComp.sprite->GetSourceRect();
                bounds.width = static_cast<float>(src.width) * spriteComp.scaleX;
                bounds.height = static_cast<float>(src.height) * spriteComp.scaleY;
                bounds.offsetX = spriteComp.offsetX;
                bounds.offsetY = spriteComp.offsetY;
                return bounds;
            }
        }
    }
    return bounds;
}

static void ApplyPosition(Registry& registry, Entity entity, float x, float y) {
    if (!registry.HasComponent<Position>(entity)) {
        registry.AddComponent<Position>(entity, Position{x, y, 0});
    } else {
        // Keeps the prefab's child parts attached when the root is placed.
        HierarchySystem::TeleportWithChildren(registry, entity, x, y);
    }
}

std::vector<Entity> SpawnWaveFormation(Registry& registry, const WaveConfig& config) {
    std::vector<Entity> spawned;
    // Load prefab spawner resource
    auto& spawner = registry.GetResource<PrefabSpawner>();
    float baseX = 0.f;
    float baseY = 0.f;

    // Compute per-pattern offsets
    for (unsigned int i = 0; i < config.count; ++i) {
        Entity e = spawner.Spawn(registry, config.prefabPath);
        // Determine offset for this entity based on pattern
        float offsetX = 0.f, offsetY = 0.f;
        switch (config.pattern) {
            case WavePatternType::Line:
                offsetX = i * config.spacing;
                offsetY = 0.f;
                break;
            case WavePatternType::VFormation:
                offsetX = i * config.spacing;
                offsetY = (i % 2 == 0 ? -1 : 1) * (i / 2) * config.spacing;
                break;
            case WavePatternType::Sine:
                offsetX = i * config.spacing;
                offsetY = std::sin(i * 0.5f) * config.spacing * 2.f;
                break;
            case WavePatternType::Grid:
                offsetX = i % static_cast<unsigned int>(std::sqrt(config.count)) * config.spacing;
                offsetY = i / static_cast<unsigned int>(std::sqrt(config.count)) * config.spacing;
                break;
            default:
                offsetX = i * config.spacing;
                offsetY = 0.f;
                break;
        }
        ApplyPosition(registry, e, baseX + offsetX, baseY + offsetY);
        spawned.push_back(e);
    }

    if (config.attachParent) {
        // Create dummy parent entity
        Entity parent = registry.CreateEntity();
        // Add Parent component to children
        for (auto child : spawned) {
            if (!registry.HasComponent<Parent>(child)) {
                registry.AddComponent<Parent>(child, Parent{parent});
            }
        }
        // Optionally attach a script to the parent for group behavior
        if (!config.parentScriptPath.empty()) {
            if (!registry.HasComponent<ScriptComponent>(parent)) {
                ScriptComponent sc;
                sc.scriptPath = config.parentScriptPath;
                registry.AddComponent<ScriptComponent>(parent, std::move(sc));
            } else {
                auto& script = registry.RequireComponent<ScriptComponent>(parent);
                script.scriptPath = config.parentScriptPath;
            }
        }
    }

    return spawned;
}

} // namespace Engine