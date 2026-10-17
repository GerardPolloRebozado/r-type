#include "Panels/SceneViewPanel.hpp"
#include "Components/ColliderComponent.hpp"
#include "Components/Position.hpp"
#include "Components/Rotation.hpp"
#include "Components/ShapeComponent.hpp"
#include "EditorConfig.hpp"
#include "Panels/HierarchyPanel.hpp"
#include "Systems/CameraSystem.hpp"
#include <algorithm>
#include <cmath>
#include <imgui.h>
#include <rlImGui.h>

namespace Editor {

namespace {
constexpr float kGameViewFillRatio = 0.7f;
constexpr float kMinEditorZoom = 0.1f;
constexpr float kMaxEditorZoom = 10.0f;
constexpr ImU32 kGameViewFillColor = IM_COL32(120, 200, 255, 16);
constexpr ImU32 kGameViewBorderColor = IM_COL32(140, 210, 255, 200);
} // namespace

void SceneViewPanel::InitEditorCamera(Engine::Registry& registry, float viewWidth,
                                      float viewHeight) {
    if (m_EditorCameraInitialized || viewWidth <= 0.0f || viewHeight <= 0.0f ||
        !registry.HasResource<Engine::VirtualResolution>()) {
        return;
    }

    const auto& canvas = registry.GetResource<Engine::VirtualResolution>();
    const float width = static_cast<float>(canvas.GetWidth());
    const float height = static_cast<float>(canvas.GetHeight());
    if (width <= 0.0f || height <= 0.0f) {
        return;
    }

    const Camera2D gameCamera = Engine::CameraSystem::GetActiveCamera(registry, width, height);
    const Vector2 corners[4] = {GetScreenToWorld2D({0.0f, 0.0f}, gameCamera),
                                GetScreenToWorld2D({width, 0.0f}, gameCamera),
                                GetScreenToWorld2D({width, height}, gameCamera),
                                GetScreenToWorld2D({0.0f, height}, gameCamera)};

    float minX = corners[0].x;
    float maxX = corners[0].x;
    float minY = corners[0].y;
    float maxY = corners[0].y;
    for (const Vector2& corner : corners) {
        minX = std::min(minX, corner.x);
        maxX = std::max(maxX, corner.x);
        minY = std::min(minY, corner.y);
        maxY = std::max(maxY, corner.y);
    }

    const float extentX = std::max(maxX - minX, 1.0f);
    const float extentY = std::max(maxY - minY, 1.0f);
    const float fit = std::min(viewWidth / extentX, viewHeight / extentY);

    m_EditorCamera.offset = {viewWidth * 0.5f, viewHeight * 0.5f};
    m_EditorCamera.rotation = 0.0f;
    m_EditorCamera.target = {(minX + maxX) * 0.5f, (minY + maxY) * 0.5f};
    m_EditorCamera.zoom = std::clamp(kGameViewFillRatio * fit, kMinEditorZoom, kMaxEditorZoom);
    m_EditorCameraInitialized = true;
}

std::array<Vector2, 4> SceneViewPanel::GameViewportInTarget(Engine::Registry& registry,
                                                            int canvasWidth,
                                                            int canvasHeight) const {
    const float width = static_cast<float>(canvasWidth);
    const float height = static_cast<float>(canvasHeight);
    const Camera2D gameCamera = Engine::CameraSystem::GetActiveCamera(registry, width, height);
    // Canvas pixels -> world (game camera) -> render-target pixels (editor camera)
    auto canvasToTarget = [&](float x, float y) {
        const Vector2 world = GetScreenToWorld2D({x, y}, gameCamera);
        return GetWorldToScreen2D(world, m_EditorCamera);
    };
    return {canvasToTarget(0.0f, 0.0f), canvasToTarget(width, 0.0f), canvasToTarget(width, height),
            canvasToTarget(0.0f, height)};
}

std::optional<Camera2D> SceneViewPanel::GameScreenCamera(Engine::Registry& registry) const {
    if (!registry.HasResource<Engine::VirtualResolution>()) {
        return std::nullopt;
    }
    const auto& canvas = registry.GetResource<Engine::VirtualResolution>();
    const int canvasWidth = canvas.GetWidth();
    const int canvasHeight = canvas.GetHeight();
    if (canvasWidth <= 0 || canvasHeight <= 0) {
        return std::nullopt;
    }

    const std::array<Vector2, 4> quad = GameViewportInTarget(registry, canvasWidth, canvasHeight);
    float minX = quad[0].x;
    float maxX = quad[0].x;
    float minY = quad[0].y;
    float maxY = quad[0].y;
    for (const Vector2& corner : quad) {
        minX = std::min(minX, corner.x);
        maxX = std::max(maxX, corner.x);
        minY = std::min(minY, corner.y);
        maxY = std::max(maxY, corner.y);
    }
    if (maxX <= minX || maxY <= minY) {
        return std::nullopt;
    }

    Camera2D screenCamera{
        {0.0f, 0.0f}, {minX, minY}, 0.0f, (maxX - minX) / static_cast<float>(canvasWidth)};
    return screenCamera;
}

void SceneViewPanel::RenderGame(Engine::Registry& registry, EditorConfig& config) {
    const Vector2 viewport = m_ViewportSize;
    if (viewport.x <= 0.0f || viewport.y <= 0.0f) {
        Vector2 fallback{1280.0f, 720.0f};
        if (registry.HasResource<Engine::VirtualResolution>()) {
            const auto& canvas = registry.GetResource<Engine::VirtualResolution>();
            fallback = {static_cast<float>(canvas.GetWidth()),
                        static_cast<float>(canvas.GetHeight())};
        }
        m_Renderer.Render(registry, config, fallback, m_EditorCamera);
        return;
    }

    if (!m_EditorCameraInitialized) {
        InitEditorCamera(registry, viewport.x, viewport.y);
    }
    m_EditorCamera.offset = {viewport.x * 0.5f, viewport.y * 0.5f};

    m_Renderer.Render(registry, config, viewport, m_EditorCamera, GameScreenCamera(registry));
}

static bool IsPointInRect(const ImVec2& pt, const ImVec2& min, const ImVec2& max) {
    return pt.x >= min.x && pt.x <= max.x && pt.y >= min.y && pt.y <= max.y;
}

void SceneViewPanel::OnImGuiRender(Engine::Registry& registry, EditorConfig& config) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    if (ImGui::Begin(m_Name.c_str())) {
        ImVec2 viewportMinRegion = ImGui::GetWindowContentRegionMin();
        ImVec2 viewportMaxRegion = ImGui::GetWindowContentRegionMax();
        ImVec2 viewportOffset = ImGui::GetWindowPos();

        ImVec2 viewportBoundsMin =
            ImVec2(viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y);
        ImVec2 viewportBoundsMax =
            ImVec2(viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y);
        ImVec2 viewportSize = ImVec2(viewportMaxRegion.x - viewportMinRegion.x,
                                     viewportMaxRegion.y - viewportMinRegion.y);
        // The scene is rendered before the UI, so this size drives the next frame's
        // render target (and the initial framing)
        m_ViewportSize = Vector2{viewportSize.x, viewportSize.y};

        // Draw the Game Texture across the whole tab: the scene view is the viewport.
        const auto& targetTexture = m_Renderer.GetTargetTexture();
        if (targetTexture.id == 0) {
            ImGui::End();
            ImGui::PopStyleVar();
            return;
        }
        const ImVec2 drawSize = viewportSize;

        rlImGuiImageRect(&targetTexture.texture, static_cast<int>(drawSize.x),
                         static_cast<int>(drawSize.y),
                         Rectangle{0, 0, static_cast<float>(targetTexture.texture.width),
                                   -static_cast<float>(targetTexture.texture.height)});

        ImVec2 textureScreenPos = ImGui::GetItemRectMin();
        float scaleX = drawSize.x / static_cast<float>(targetTexture.texture.width);
        float scaleY = drawSize.y / static_cast<float>(targetTexture.texture.height);

        const Camera2D camera = m_EditorCamera;
        auto worldToView = [&](const Camera2D& cam, float x, float y) {
            const Vector2 px = GetWorldToScreen2D({x, y}, cam);
            return ImVec2(textureScreenPos.x + px.x * scaleX, textureScreenPos.y + px.y * scaleY);
        };
        auto viewToWorld = [&](const Camera2D& cam, float x, float y) {
            return GetScreenToWorld2D(
                {(x - textureScreenPos.x) / scaleX, (y - textureScreenPos.y) / scaleY}, cam);
        };
        if (ImGui::IsItemHovered()) {
            const float wheel = ImGui::GetIO().MouseWheel;
            if (wheel != 0.0f) {
                const ImVec2 mouse = ImGui::GetMousePos();
                const Vector2 before = viewToWorld(m_EditorCamera, mouse.x, mouse.y);
                m_EditorCamera.zoom = std::clamp(m_EditorCamera.zoom * std::pow(1.1f, wheel),
                                                 kMinEditorZoom, kMaxEditorZoom);
                const Vector2 after = viewToWorld(m_EditorCamera, mouse.x, mouse.y);
                m_EditorCamera.target.x += before.x - after.x;
                m_EditorCamera.target.y += before.y - after.y;
            }
            if (ImGui::IsMouseDragging(ImGuiMouseButton_Right) ||
                ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) {
                ImVec2 delta = ImGui::GetIO().MouseDelta;
                m_EditorCamera.target.x -= (delta.x / scaleX) / m_EditorCamera.zoom;
                m_EditorCamera.target.y -= (delta.y / scaleY) / m_EditorCamera.zoom;
            }
        }

        if (registry.HasResource<Engine::VirtualResolution>()) {
            const auto& canvas = registry.GetResource<Engine::VirtualResolution>();
            const int canvasWidth = canvas.GetWidth();
            const int canvasHeight = canvas.GetHeight();
            if (canvasWidth > 0 && canvasHeight > 0) {
                const std::array<Vector2, 4> gameViewportPixels =
                    GameViewportInTarget(registry, canvasWidth, canvasHeight);
                ImVec2 gameView[4];
                for (size_t i = 0; i < gameViewportPixels.size(); ++i) {
                    gameView[i] = ImVec2(textureScreenPos.x + gameViewportPixels[i].x * scaleX,
                                         textureScreenPos.y + gameViewportPixels[i].y * scaleY);
                }
                ImDrawList* overlay = ImGui::GetWindowDrawList();
                overlay->AddConvexPolyFilled(gameView, 4, kGameViewFillColor);
                overlay->AddPolyline(gameView, 4, kGameViewBorderColor, 2.0f, ImDrawFlags_Closed);
            }
        }
        if (registry.HasResource<HierarchyPanelSelectedEntity>()) {
            auto& selected = registry.GetResource<HierarchyPanelSelectedEntity>();
            if (selected.entity != -1 && registry.HasComponent<Engine::Position>(selected.entity)) {
                auto& pos = registry.GetComponent<Engine::Position>(selected.entity);

                // Map game coordinates to screen coordinates inside the Scene view
                ImVec2 entityScreenPos = worldToView(camera, pos.x, pos.y);

                ImDrawList* drawList = ImGui::GetWindowDrawList();

                float arrowLen = 80.0f;
                float arrowThickness = 4.0f;
                float headSize = 10.0f;

                ImVec2 xEnd = ImVec2(entityScreenPos.x + arrowLen, entityScreenPos.y);
                ImVec2 yEnd = ImVec2(entityScreenPos.x, entityScreenPos.y + arrowLen);

                // Define hitboxes
                ImVec2 centerMin = ImVec2(entityScreenPos.x - 10, entityScreenPos.y - 10);
                ImVec2 centerMax = ImVec2(entityScreenPos.x + 10, entityScreenPos.y + 10);

                ImVec2 xMin = ImVec2(entityScreenPos.x + 10, entityScreenPos.y - 10);
                ImVec2 xMax =
                    ImVec2(entityScreenPos.x + arrowLen + headSize, entityScreenPos.y + 10);

                ImVec2 yMin = ImVec2(entityScreenPos.x - 10, entityScreenPos.y + 10);
                ImVec2 yMax =
                    ImVec2(entityScreenPos.x + 10, entityScreenPos.y + arrowLen + headSize);

                ImVec2 mousePos = ImGui::GetMousePos();
                bool hoveringX = IsPointInRect(mousePos, xMin, xMax);
                bool hoveringY = IsPointInRect(mousePos, yMin, yMax);
                bool hoveringCenter = IsPointInRect(mousePos, centerMin, centerMax);

                // Check rotation ring hitbox
                float rotRingRadius = 60.0f;
                float rotRingThickness = 5.0f;
                float distToCenter =
                    std::hypot(mousePos.x - entityScreenPos.x, mousePos.y - entityScreenPos.y);
                bool hoveringRot = false;
                if (registry.HasComponent<Engine::Rotation>(selected.entity)) {
                    if (distToCenter >= rotRingRadius - rotRingThickness &&
                        distToCenter <= rotRingRadius + rotRingThickness) {
                        hoveringRot = true;
                    }
                }

                if (ImGui::IsWindowHovered()) {
                    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                        m_DragCamera = camera;
                        if (hoveringRot) {
                            m_GizmoState = GizmoState::DraggingRotation;
                            // Calculate initial angle to mouse
                            float dx = mousePos.x - entityScreenPos.x;
                            float dy = mousePos.y - entityScreenPos.y;
                            auto& rot = registry.GetComponent<Engine::Rotation>(selected.entity);
                            m_InitialDragAngle = std::atan2(dy, dx) * (180.0f / 3.1415926535f);
                            m_InitialEntityAngle = rot.angle;
                        } else if (hoveringCenter) {
                            m_GizmoState = GizmoState::DraggingBoth;
                            m_DragOffset[0] = entityScreenPos.x - mousePos.x;
                            m_DragOffset[1] = entityScreenPos.y - mousePos.y;
                        } else if (hoveringX) {
                            m_GizmoState = GizmoState::DraggingX;
                            m_DragOffset[0] = entityScreenPos.x - mousePos.x;
                        } else if (hoveringY) {
                            m_GizmoState = GizmoState::DraggingY;
                            m_DragOffset[1] = entityScreenPos.y - mousePos.y;
                        } else {
                            m_GizmoState = GizmoState::None;
                        }
                    }
                }

                if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                    m_GizmoState = GizmoState::None;
                }

                if (m_GizmoState == GizmoState::DraggingBoth) {
                    const Vector2 world = viewToWorld(m_DragCamera, mousePos.x + m_DragOffset[0],
                                                      mousePos.y + m_DragOffset[1]);
                    pos.x = world.x;
                    pos.y = world.y;
                } else if (m_GizmoState == GizmoState::DraggingX) {
                    pos.x = viewToWorld(m_DragCamera, mousePos.x + m_DragOffset[0], mousePos.y).x;
                } else if (m_GizmoState == GizmoState::DraggingY) {
                    pos.y = viewToWorld(m_DragCamera, mousePos.x, mousePos.y + m_DragOffset[1]).y;
                } else if (m_GizmoState == GizmoState::DraggingRotation) {
                    float dx = mousePos.x - entityScreenPos.x;
                    float dy = mousePos.y - entityScreenPos.y;
                    float currentMouseAngle = std::atan2(dy, dx) * (180.0f / 3.1415926535f);
                    float deltaAngle = currentMouseAngle - m_InitialDragAngle;
                    auto& rot = registry.GetComponent<Engine::Rotation>(selected.entity);
                    rot.angle = m_InitialEntityAngle + deltaAngle;
                    // Normalize to 0-360
                    while (rot.angle < 0.0f) {
                        rot.angle += 360.0f;
                    }
                    while (rot.angle >= 360.0f) {
                        rot.angle -= 360.0f;
                    }
                }

                entityScreenPos = worldToView(camera, pos.x, pos.y);
                xEnd = ImVec2(entityScreenPos.x + arrowLen, entityScreenPos.y);
                yEnd = ImVec2(entityScreenPos.x, entityScreenPos.y + arrowLen);

                // Colors
                ImU32 colX = (hoveringX || m_GizmoState == GizmoState::DraggingX)
                                 ? IM_COL32(255, 100, 100, 255)
                                 : IM_COL32(200, 50, 50, 255);
                ImU32 colY = (hoveringY || m_GizmoState == GizmoState::DraggingY)
                                 ? IM_COL32(100, 255, 100, 255)
                                 : IM_COL32(50, 200, 50, 255);
                ImU32 colC = (hoveringCenter || m_GizmoState == GizmoState::DraggingBoth)
                                 ? IM_COL32(255, 255, 100, 255)
                                 : IM_COL32(200, 200, 50, 255);

                // Draw X Arrow (Red)
                drawList->AddLine(entityScreenPos, xEnd, colX, arrowThickness);
                drawList->AddTriangleFilled(ImVec2(xEnd.x + headSize, xEnd.y),
                                            ImVec2(xEnd.x, xEnd.y - headSize * 0.5f),
                                            ImVec2(xEnd.x, xEnd.y + headSize * 0.5f), colX);

                // Draw Y Arrow (Green)
                drawList->AddLine(entityScreenPos, yEnd, colY, arrowThickness);
                drawList->AddTriangleFilled(ImVec2(yEnd.x, yEnd.y + headSize),
                                            ImVec2(yEnd.x - headSize * 0.5f, yEnd.y),
                                            ImVec2(yEnd.x + headSize * 0.5f, yEnd.y), colY);

                // Draw Center Square (Yellow)
                drawList->AddRectFilled(ImVec2(entityScreenPos.x - 5, entityScreenPos.y - 5),
                                        ImVec2(entityScreenPos.x + 5, entityScreenPos.y + 5), colC);

                // Draw Rotation Ring (Blue)
                if (registry.HasComponent<Engine::Rotation>(selected.entity)) {
                    ImU32 colRot = (hoveringRot || m_GizmoState == GizmoState::DraggingRotation)
                                       ? IM_COL32(100, 100, 255, 255)
                                       : IM_COL32(50, 50, 200, 255);
                    drawList->AddCircle(entityScreenPos, rotRingRadius, colRot, 32,
                                        rotRingThickness);

                    // Draw a notch to indicate current angle, as seen through the camera
                    auto& rot = registry.GetComponent<Engine::Rotation>(selected.entity);
                    float angleRad = (rot.angle + camera.rotation) * (3.1415926535f / 180.0f);
                    ImVec2 notchStart = ImVec2(
                        entityScreenPos.x + std::cos(angleRad) * (rotRingRadius - rotRingThickness),
                        entityScreenPos.y +
                            std::sin(angleRad) * (rotRingRadius - rotRingThickness));
                    ImVec2 notchEnd =
                        ImVec2(entityScreenPos.x +
                                   std::cos(angleRad) * (rotRingRadius + rotRingThickness + 5.0f),
                               entityScreenPos.y +
                                   std::sin(angleRad) * (rotRingRadius + rotRingThickness + 5.0f));
                    drawList->AddLine(notchStart, notchEnd, IM_COL32(255, 255, 255, 255), 3.0f);
                }
            }
        }
    }
    ImGui::End();
    ImGui::PopStyleVar();
}

} // namespace Editor

namespace Editor {
const std::string& SceneViewPanel::GetName() const {
    return m_Name;
}
} // namespace Editor
