#include "Panels/SceneViewPanel.hpp"
#include "Components/ColliderComponent.hpp"
#include "Components/Position.hpp"
#include "Components/Rotation.hpp"
#include "Components/ShapeComponent.hpp"
#include "EditorConfig.hpp"
#include "Panels/HierarchyPanel.hpp"
#include "Systems/CameraSystem.hpp"
#include <imgui.h>
#include <rlImGui.h>

namespace Editor {

void SceneViewPanel::RenderGame(Engine::Registry& registry, EditorConfig& config) {
    m_Renderer.Render(registry, config);
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

        // Draw the Game Texture
        const auto& targetTexture = m_Renderer.GetTargetTexture(registry);
        if (targetTexture.id == 0) {
            ImGui::End();
            ImGui::PopStyleVar();
            return;
        }

        // We calculate aspect ratio to center it
        float aspect = static_cast<float>(targetTexture.texture.width) /
                       static_cast<float>(targetTexture.texture.height);
        ImVec2 drawSize = viewportSize;
        if (drawSize.x / aspect <= drawSize.y) {
            drawSize.y = drawSize.x / aspect;
        } else {
            drawSize.x = drawSize.y * aspect;
        }

        ImVec2 cursor = ImGui::GetCursorPos();
        cursor.x += (viewportSize.x - drawSize.x) * 0.5f;
        cursor.y += (viewportSize.y - drawSize.y) * 0.5f;
        ImGui::SetCursorPos(cursor);

        rlImGuiImageRect(&targetTexture.texture, static_cast<int>(drawSize.x),
                         static_cast<int>(drawSize.y),
                         Rectangle{0, 0, static_cast<float>(targetTexture.texture.width),
                                   -static_cast<float>(targetTexture.texture.height)});

        ImVec2 textureScreenPos = ImGui::GetItemRectMin();
        float scaleX = drawSize.x / static_cast<float>(targetTexture.texture.width);
        float scaleY = drawSize.y / static_cast<float>(targetTexture.texture.height);

        const Camera2D camera = Engine::CameraSystem::GetActiveCamera(
            registry, static_cast<float>(targetTexture.texture.width),
            static_cast<float>(targetTexture.texture.height));
        auto worldToView = [&](const Camera2D& cam, float x, float y) {
            const Vector2 px = GetWorldToScreen2D({x, y}, cam);
            return ImVec2(textureScreenPos.x + px.x * scaleX, textureScreenPos.y + px.y * scaleY);
        };
        auto viewToWorld = [&](const Camera2D& cam, float x, float y) {
            return GetScreenToWorld2D(
                {(x - textureScreenPos.x) / scaleX, (y - textureScreenPos.y) / scaleY}, cam);
        };

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
