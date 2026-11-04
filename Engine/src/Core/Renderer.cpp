#include "Core/Renderer.hpp"
#include "Components/Position.hpp"
#include "ECS/LayeredRenderSystem.hpp"
#include "Systems/CameraSystem.hpp"
#include "Systems/ReplicationSystem.hpp"
#include <algorithm>
#include <raylib.h>
#include <tuple>
#include <vector>

namespace Engine {
namespace {
struct LayeredDraw {
    int z;
    size_t systemOrder;
    Entity entity;
    LayeredRenderSystem* system;
};

// Draws every entity of every LayeredRenderSystem for one pass, sorted by
// (z, system registration order, entity id).
void DrawLayered(Registry& registry, bool screenSpace) {
    auto const& systems = registry.GetSystems();
    std::vector<LayeredRenderSystem*> layeredSystems;
    for (std::type_index const& systemId : registry.GetSystemRenderOrder()) {
        if (auto* layered = dynamic_cast<LayeredRenderSystem*>(systems.at(systemId).get())) {
            layeredSystems.push_back(layered);
        }
    }
    if (layeredSystems.empty()) {
        return;
    }

    std::vector<LayeredDraw> draws;
    for (size_t i = 0; i < registry.GetEntityCount(); ++i) {
        Entity entity{i};
        if (!registry.HasComponent<Position>(entity)) {
            continue;
        }
        int const z = registry.RequireComponent<Position>(entity).z;
        for (size_t order = 0; order < layeredSystems.size(); ++order) {
            if (layeredSystems[order]->CanDraw(registry, entity, screenSpace)) {
                draws.push_back({z, order, entity, layeredSystems[order]});
            }
        }
    }

    std::sort(draws.begin(), draws.end(), [](LayeredDraw const& a, LayeredDraw const& b) {
        return std::tie(a.z, a.systemOrder, a.entity) < std::tie(b.z, b.systemOrder, b.entity);
    });

    for (LayeredDraw const& draw : draws) {
        draw.system->DrawEntity(registry, draw.entity);
    }
}
} // namespace

void RenderScene(Registry& registry, float viewportWidth, float viewportHeight,
                 const std::function<void()>& worldOverlay, std::optional<Camera2D> cameraOverride,
                 std::optional<Camera2D> screenCameraOverride) {
    auto const& systems = registry.GetSystems();

    if (cameraOverride) {
        BeginMode2D(*cameraOverride);
    } else {
        BeginMode2D(CameraSystem::GetActiveCamera(registry, viewportWidth, viewportHeight));
    }
    // Interpolate replicated transforms before drawing so 20 Hz snapshots
    // read as smooth motion at any frame rate.
    if (registry.GetSystems().contains(typeid(Engine::Multiplayer::ReplicationSystem))) {
        auto* repl = dynamic_cast<Engine::Multiplayer::ReplicationSystem*>(
            registry.GetSystems().at(typeid(Engine::Multiplayer::ReplicationSystem)).get());
        if (repl) {
            repl->Interpolate(registry);
        }
    }
    DrawLayered(registry, false);

    for (std::type_index const& systemId : registry.GetSystemRenderOrder()) {
        systems.at(systemId)->Render(registry);
    }
    if (worldOverlay) {
        worldOverlay();
    }
    EndMode2D();

    if (screenCameraOverride) {
        BeginMode2D(*screenCameraOverride);
    }
    DrawLayered(registry, true);
    for (std::type_index const& systemId : registry.GetSystemRenderOrder()) {
        systems.at(systemId)->RenderScreen(registry);
    }
    if (screenCameraOverride) {
        EndMode2D();
    }
}

} // namespace Engine
