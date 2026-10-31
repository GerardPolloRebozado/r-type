#include "Systems/WaveSpawnerSystem.hpp"
#include "Components/CameraComponent.hpp"
#include "Components/Parent.hpp"
#include "Components/Position.hpp"
#include "ECS/Registry.hpp"
#include "Logging/Logger.hpp"
#include "Resources/PrefabSpawner.hpp"
#include "Resources/WaveSchedule.hpp"
#include "Utils/WavePatterns.hpp"
#include <optional>

namespace Engine {
namespace {

std::optional<Entity> FindActiveCamera(Registry& registry) {
    if (!registry.IsComponentRegistered<CameraComponent>()) {
        return std::nullopt;
    }
    std::optional<Entity> active;
    for (const auto& [entity, camera] : *registry.GetComponentArray<CameraComponent>()) {
        if (camera.primary && registry.HasComponent<Position>(entity) &&
            (!active || entity < *active)) {
            active = entity;
        }
    }
    return active;
}

void SetPosition(Registry& registry, Entity entity, float x, float y) {
    if (!registry.HasComponent<Position>(entity)) {
        return;
    }
    auto& position = registry.RequireComponent<Position>(entity);
    position.x = x;
    position.y = y;
    position.prevX = x;
    position.prevY = y;
    position.moved = true;
}

} // namespace

void WaveSpawnerSystem::Update(Registry& registry, float dt) {
    if (!registry.HasResource<WaveSchedule>() || !registry.HasResource<PrefabSpawner>()) {
        return;
    }

    auto& schedule = registry.GetResource<WaveSchedule>();
    if (schedule.waves.empty()) {
        return;
    }
    if (schedule.spawned.size() != schedule.waves.size()) {
        schedule.spawned.assign(schedule.waves.size(), 0);
        m_elapsed = 0.0f;
    }

    float camX = 0.0f;
    const std::optional<Entity> camera = FindActiveCamera(registry);
    if (camera) {
        camX = registry.RequireComponent<Position>(*camera).x;
    } else {
        // No scene camera (tests, prototypes): fall back to the old time-based
        // camera simulation so waves still trigger.
        m_elapsed += dt;
        camX = -400.0f + m_elapsed * 60.0f;
    }

    for (std::size_t i = 0; i < schedule.waves.size(); ++i) {
        if (schedule.spawned[i] != 0) {
            continue;
        }
        const WaveDefinition& wave = schedule.waves[i];
        if (camX < wave.triggerX) {
            continue;
        }
        schedule.spawned[i] = 1;

        log_info("[WaveSpawner] Spawning " + wave.name + " at camX=" + std::to_string(camX));

        WaveConfig config{};
        config.pattern = wave.formation;
        config.prefabPath = wave.prefabPath;
        config.count = wave.count;
        config.spacing = wave.spacing;
        config.attachParent = wave.attachParent;
        config.parentScriptPath = wave.groupScriptPath;

        const std::vector<Entity> spawned = SpawnWaveFormation(registry, config);
        const float startX = camX + wave.spawnOffsetX;
        for (const Entity child : spawned) {
            if (!registry.HasComponent<Position>(child)) {
                continue;
            }
            const Position position = registry.RequireComponent<Position>(child);
            SetPosition(registry, child, startX + position.x, wave.baseY + position.y);
        }

        if (wave.attachParent && !spawned.empty() &&
            registry.HasComponent<Parent>(spawned.front())) {
            const Entity parent = registry.RequireComponent<Parent>(spawned.front()).parent;
            SetPosition(registry, parent, startX, wave.baseY);
        }
    }
}

} // namespace Engine