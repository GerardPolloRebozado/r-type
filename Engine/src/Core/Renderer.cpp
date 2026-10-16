#include "Core/Renderer.hpp"
#include "Systems/CameraSystem.hpp"
#include <raylib.h>

namespace Engine {
void RenderScene(Registry& registry, float viewportWidth, float viewportHeight,
                 const std::function<void()>& worldOverlay,
                 std::optional<Camera2D> cameraOverride) {
    auto const& systems = registry.GetSystems();
    if (cameraOverride) {
        BeginMode2D(*cameraOverride);
    } else {
        BeginMode2D(CameraSystem::GetActiveCamera(registry, viewportWidth, viewportHeight));
    }
    for (std::type_index const& systemId : registry.GetSystemRenderOrder()) {
        systems.at(systemId)->Render(registry);
    }
    if (worldOverlay) {
        worldOverlay();
    }
    EndMode2D();

    for (std::type_index const& systemId : registry.GetSystemRenderOrder()) {
        systems.at(systemId)->RenderScreen(registry);
    }
}
} // namespace Engine
