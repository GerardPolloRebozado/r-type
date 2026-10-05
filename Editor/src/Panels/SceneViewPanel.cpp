#include "Panels/SceneViewPanel.hpp"
#include "Components/Position.hpp"
#include "EditorConfig.hpp"
#include "Panels/HierarchyPanel.hpp"
#include <imgui.h>
#include <rlImGui.h>

namespace Editor {

SceneViewPanel::SceneViewPanel() {
    // Texture initialized lazily in RenderGame after OpenGL context is created
}

SceneViewPanel::~SceneViewPanel() {
    m_GameTexture.Unload();
}

void SceneViewPanel::RenderGame(Engine::Registry& registry, EditorConfig* config) {

    if (m_GameTexture.id == 0) {
        m_GameTexture = raylib::RenderTexture2D(config->windowWidth, config->windowHeight);
    }
    bool hasWidthChanged = m_GameTexture.GetTexture().GetSize().x;
    bool hasHeightChanged = m_GameTexture.GetTexture().GetSize().y;
    if (hasWidthChanged || hasHeightChanged) {
        m_GameTexture = raylib::RenderTexture2D(config->windowWidth, config->windowHeight);
    }
    m_GameTexture.BeginMode();
    ClearBackground(BLACK);
    for (auto const& pair : registry.GetSystems()) {
        pair.second->Render(registry);
    }
    m_GameTexture.EndMode();
}

static bool IsPointInRect(const ImVec2& pt, const ImVec2& min, const ImVec2& max) {
    return pt.x >= min.x && pt.x <= max.x && pt.y >= min.y && pt.y <= max.y;
}

void SceneViewPanel::OnImGuiRender(Engine::Registry* registry, EditorConfig*) {
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
        // We calculate aspect ratio to center it
        float aspect = (float)m_GameTexture.texture.width / (float)m_GameTexture.texture.height;
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

        rlImGuiImageRect(&m_GameTexture.texture, (int)drawSize.x, (int)drawSize.y,
                         Rectangle{0, 0, (float)m_GameTexture.texture.width,
                                   -(float)m_GameTexture.texture.height});

        ImVec2 textureScreenPos = ImGui::GetItemRectMin();
        float scaleX = drawSize.x / (float)m_GameTexture.texture.width;
        float scaleY = drawSize.y / (float)m_GameTexture.texture.height;

        if (registry && registry->HasResource<HierarchyPanelSelectedEntity>()) {
            auto& selected = registry->GetResource<HierarchyPanelSelectedEntity>();
            if (selected.entity != -1 &&
                registry->HasComponent<Engine::Position>(selected.entity)) {
                auto& pos = registry->GetComponent<Engine::Position>(selected.entity);

                // Map game coordinates to screen coordinates inside the Scene view
                ImVec2 entityScreenPos = ImVec2(textureScreenPos.x + pos.x * scaleX,
                                                textureScreenPos.y + pos.y * scaleY);

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

                if (ImGui::IsWindowHovered()) {
                    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                        if (hoveringCenter) {
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
                    pos.x = (mousePos.x + m_DragOffset[0] - textureScreenPos.x) / scaleX;
                    pos.y = (mousePos.y + m_DragOffset[1] - textureScreenPos.y) / scaleY;
                } else if (m_GizmoState == GizmoState::DraggingX) {
                    pos.x = (mousePos.x + m_DragOffset[0] - textureScreenPos.x) / scaleX;
                } else if (m_GizmoState == GizmoState::DraggingY) {
                    pos.y = (mousePos.y + m_DragOffset[1] - textureScreenPos.y) / scaleY;
                }

                entityScreenPos = ImVec2(textureScreenPos.x + pos.x * scaleX,
                                         textureScreenPos.y + pos.y * scaleY);
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
