#include "Rendering/SceneRenderer.hpp"
#include "Components/ColliderComponent.hpp"
#include "Components/Position.hpp"
#include "Components/Rotation.hpp"
#include "Components/ShapeComponent.hpp"
#include "Core/Renderer.hpp"
#include "Panels/HierarchyPanel.hpp"
#include <optional>

namespace Editor {

void SceneRenderer::Render(Engine::Registry& registry, const EditorConfig& config,
                           std::optional<Camera2D> editorCamera) {
    int virtualWidth =
        Engine::VirtualResolution::ResolveAxis(config.virtualWidth, config.windowWidth);
    int virtualHeight =
        Engine::VirtualResolution::ResolveAxis(config.virtualHeight, config.windowHeight);
    auto& canvas = registry.GetResource<Engine::VirtualResolution>();

    canvas.EnsureSize(virtualWidth, virtualHeight);
    canvas.Begin();
    ClearBackground(BLACK);

    RenderSystems(registry, editorCamera);

    canvas.End();
}

void SceneRenderer::RenderSystems(Engine::Registry& registry,
                                  std::optional<Camera2D> editorCamera) {
    const auto& canvas = registry.GetResource<Engine::VirtualResolution>();

    Engine::RenderScene(
        registry, static_cast<float>(canvas.GetWidth()), static_cast<float>(canvas.GetHeight()),
        [this, &registry]() { RenderColliderOverlays(registry); }, editorCamera);
}

void SceneRenderer::RenderColliderOverlays(Engine::Registry& registry) {
    try {
        auto colliders = registry.GetComponentArray<Engine::ColliderComponent>();
        static const Engine::Rotation defaultRot{0.0f};

        Engine::Entity selectedEntity = -1;
        if (registry.HasResource<HierarchyPanelSelectedEntity>()) {
            selectedEntity = registry.GetResource<HierarchyPanelSelectedEntity>().entity;
        }

        for (const auto& [entity, col] : *colliders) {
            if (!col.enabled || !registry.HasComponent<Engine::Position>(entity)) {
                continue;
            }
            const auto& pos = registry.GetComponent<Engine::Position>(entity);
            const auto& rot = registry.HasComponent<Engine::Rotation>(entity)
                                  ? registry.GetComponent<Engine::Rotation>(entity)
                                  : defaultRot;

            std::optional<Engine::ShapeComponent> shape = col.shape;
            if (!shape.has_value() && registry.HasComponent<Engine::ShapeComponent>(entity)) {
                shape = registry.GetComponent<Engine::ShapeComponent>(entity);
            }
            if (!shape.has_value()) {
                continue;
            }

            const auto verts = shape->GetShapeVectors(rot, pos);
            if (verts.empty()) {
                continue;
            }

            const bool isSelected = (entity == selectedEntity);
            Color outlineColor;
            if (col.isTrigger) {
                outlineColor = isSelected ? Color{255, 230, 0, 255} : Color{220, 180, 0, 180};
            } else {
                outlineColor = isSelected ? Color{50, 255, 120, 255} : Color{0, 200, 80, 180};
            }

            const float thickness = isSelected ? 3.5f : 2.0f;

            if (shape->type == Engine::ShapeType::LINE && verts.size() >= 2) {
                DrawLineEx(verts[0], verts[1], thickness, outlineColor);
            } else if (verts.size() >= 3) {
                for (size_t k = 0; k < verts.size(); ++k) {
                    size_t next = (k + 1) % verts.size();
                    DrawLineEx(verts[k], verts[next], thickness, outlineColor);
                }
            }
        }
    } catch (const Engine::ErrorEcs&) {
        // Colliders not registered
    }
}

} // namespace Editor
