#include "Rendering/SceneRenderer.hpp"
#include "Assets/AssetManager.hpp"
#include "Components/ColliderComponent.hpp"
#include "Components/Position.hpp"
#include "Components/Rotation.hpp"
#include "Components/ShapeComponent.hpp"
#include "Plugins/ColliderPlugin.hpp"
#include "Components/SpriteComponent.hpp"
#include "Core/Renderer.hpp"
#include "Panels/HierarchyPanel.hpp"
#include "PathUtils.hpp"
#include <optional>

namespace Editor {

SceneRenderer::~SceneRenderer() {
    UnloadTarget();
}

void SceneRenderer::UnloadTarget() {
    if (m_Target.id == 0) {
        return;
    }

    UnloadRenderTexture(m_Target);
    m_Target = RenderTexture2D{};
    m_Width = 0;
    m_Height = 0;
}

void SceneRenderer::EnsureTarget(int width, int height) {
    if (width <= 0 || height <= 0) {
        return;
    }
    if (m_Target.id != 0 && m_Width == width && m_Height == height) {
        return;
    }

    UnloadTarget();
    m_Width = width;
    m_Height = height;
    m_Target = LoadRenderTexture(m_Width, m_Height);
    SetTextureFilter(m_Target.texture, TEXTURE_FILTER_BILINEAR);
}

void SceneRenderer::Render(Engine::Registry& registry, const EditorConfig& config,
                           Vector2 viewportSize, std::optional<Camera2D> editorCamera,
                           std::optional<Camera2D> screenCamera) {
    if (registry.HasResource<Engine::VirtualResolution>()) {
        int virtualWidth =
            Engine::VirtualResolution::ResolveAxis(config.virtualWidth, config.windowWidth);
        int virtualHeight =
            Engine::VirtualResolution::ResolveAxis(config.virtualHeight, config.windowHeight);
        registry.GetResource<Engine::VirtualResolution>().EnsureSize(virtualWidth, virtualHeight);
    }

    for (size_t i = 0; i < registry.GetEntityCount(); ++i) {
        Engine::Entity entity = static_cast<Engine::Entity>(i);
        if (!registry.HasComponent<Engine::SpriteComponent>(entity)) {
            continue;
        }
        auto& sc = registry.GetComponent<Engine::SpriteComponent>(entity);
        if (sc.sprite || sc.assetPath.empty()) {
            continue;
        }
        try {
            auto resolved = Serialization::ResolveAssetPath(registry, sc.assetPath);
            if (registry.HasResource<Engine::AssetManager>()) {
                auto& am = registry.GetResource<Engine::AssetManager>();
                am.LoadSprite(sc.assetPath, resolved.string());
                sc.sprite = am.GetSprite(sc.assetPath);
            } else {
                sc.sprite = std::make_shared<Engine::Sprite>(resolved.string());
            }
            if (sc.sprite && sc.scaleX > 0.0f && sc.scaleY > 0.0f) {
                sc.sprite->SetScale({sc.scaleX, sc.scaleY});
            }
        } catch (...) {
        }
    }

    EnsureTarget(static_cast<int>(viewportSize.x), static_cast<int>(viewportSize.y));
    if (m_Target.id == 0) {
        return;
    }

    BeginTextureMode(m_Target);
    ClearBackground(BLACK);

    RenderSystems(registry, editorCamera, screenCamera);

    EndTextureMode();
}

void SceneRenderer::RenderSystems(Engine::Registry& registry, std::optional<Camera2D> editorCamera,
                                  std::optional<Camera2D> screenCamera) {
    Engine::RenderScene(
        registry, static_cast<float>(m_Width), static_cast<float>(m_Height),
        [this, &registry]() { RenderColliderOverlays(registry); }, editorCamera, screenCamera);
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

            std::optional<Engine::ShapeComponent> shape =
                Engine::ColliderPlugin::ResolveShape(registry, entity, col);
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
