#include "Systems/CameraSystem.hpp"
#include "Components/CameraComponent.hpp"
#include "Components/Position.hpp"
#include "Components/Rotation.hpp"
#include "ECS/Registry.hpp"
#include <algorithm>
#include <optional>
#include <random>

namespace Engine {
namespace helpers {
std::optional<Entity> FindActiveCamera(Registry& registry) {
    if (!registry.IsComponentRegistered<CameraComponent>()) {
        return std::nullopt;
    }
    // Lowest entity id wins so choice does not depend on hash map order
    std::optional<Entity> active;
    for (const auto& [entity, camera] : *registry.GetComponentArray<CameraComponent>()) {
        if (camera.primary && registry.HasComponent<Position>(entity) &&
            (!active || entity < *active)) {
            active = entity;
        }
    }
    return active;
}

// Clamps the camera center on one axis (X or Y) so the view stays inside the bounds
float ClampAxis(float center, float halfView, float boundsMin, float boundsSize) {
    if (boundsSize <= halfView * 2.0f) {
        return boundsMin + boundsSize * 0.5f;
    }
    return std::clamp(center, boundsMin + halfView, boundsMin + boundsSize - halfView);
}
} // namespace helpers

void CameraSystem::Update(Registry& registry, float dt) {
    static std::mt19937 rng{std::random_device{}()};
    std::uniform_real_distribution<float> unit(-1.0f, 1.0f);

    for (auto& [entity, camera] : *registry.GetComponentArray<CameraComponent>()) {
        if (camera.followTarget && registry.HasComponent<Position>(entity) &&
            registry.HasComponent<Position>(*camera.followTarget)) {
            auto& position = registry.GetComponent<Position>(entity);
            const auto& target = registry.GetComponent<Position>(*camera.followTarget);
            const float t = camera.followSpeed <= 0.0f ? 1.0f
                                                       : std::min(1.0f, camera.followSpeed * dt);
            position.x += (target.x - position.x) * t;
            position.y += (target.y - position.y) * t;
        }

        camera.shakeTrauma = std::max(0.0f, camera.shakeTrauma - camera.shakeDecay * dt);
        // Squared trauma feels better: small hits barely shake, big ones shake a lot.
        const float shake = camera.shakeMagnitude * camera.shakeTrauma * camera.shakeTrauma;
        camera.shakeOffsetX = shake * unit(rng);
        camera.shakeOffsetY = shake * unit(rng);
    }
}

Camera2D CameraSystem::GetActiveCamera(Registry& registry, float viewportWidth,
                                       float viewportHeight) {
    Camera2D result{{0.0f, 0.0f}, {0.0f, 0.0f}, 0.0f, 1.0f};
    const std::optional<Entity> active = helpers::FindActiveCamera(registry);
    if (!active) {
        return result;
    }

    const auto& camera = registry.GetComponent<CameraComponent>(*active);
    const auto& position = registry.GetComponent<Position>(*active);
    const float zoom = camera.zoom > 0.0f ? camera.zoom : 1.0f;

    Vector2 center{position.x, position.y};
    if (camera.clampToBounds) {
        center.x = helpers::ClampAxis(center.x, viewportWidth / (2.0f * zoom), camera.bounds.x,
                                      camera.bounds.width);
        center.y = helpers::ClampAxis(center.y, viewportHeight / (2.0f * zoom), camera.bounds.y,
                                      camera.bounds.height);
    }

    result.offset = {viewportWidth * 0.5f, viewportHeight * 0.5f};
    result.target = {center.x + camera.shakeOffsetX, center.y + camera.shakeOffsetY};
    result.rotation = registry.HasComponent<Rotation>(*active)
                          ? registry.GetComponent<Rotation>(*active).angle
                          : 0.0f;
    result.zoom = zoom;
    return result;
}

Vector2 CameraSystem::ScreenToWorld(Registry& registry, Vector2 screen) {
    return GetScreenToWorld2D(
        screen, GetActiveCamera(registry, static_cast<float>(GetScreenWidth()),
                                static_cast<float>(GetScreenHeight())));
}

Vector2 CameraSystem::WorldToScreen(Registry& registry, Vector2 world) {
    return GetWorldToScreen2D(
        world, GetActiveCamera(registry, static_cast<float>(GetScreenWidth()),
                               static_cast<float>(GetScreenHeight())));
}
} // namespace Engine
