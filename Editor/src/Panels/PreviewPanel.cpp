#include "Panels/PreviewPanel.hpp"
#include "EditorConfig.hpp"
#include <algorithm>
#include <filesystem>
#include <imgui.h>
#include <rlImGui.h>

namespace Editor {

static bool IsImageFile(const std::string& path) {
    std::string ext = std::filesystem::path(path).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".gif" ||
           ext == ".tga";
}

void PreviewPanel::OnImGuiRender(Engine::Registry*, EditorConfig* config) {
    if (config && !config->m_SelectedAssetPath.empty()) {
        m_IsOpen = true;
    }

    if (!m_IsOpen) {
        return;
    }

    if (ImGui::Begin(m_Name.c_str())) {
        if (config && !config->m_SelectedAssetPath.empty()) {
            std::string selectedPath = config->m_SelectedAssetPath;

            if (m_CurrentPreviewPath != selectedPath) {
                m_CurrentPreviewPath = selectedPath;
                if (m_PreviewTexture.id != 0) {
                    UnloadTexture(m_PreviewTexture);
                    m_PreviewTexture.id = 0;
                }

                if (IsImageFile(m_CurrentPreviewPath)) {
                    std::string ext =
                        std::filesystem::path(m_CurrentPreviewPath).extension().string();
                    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

                    if (ext == ".gif") {
                        int animFrames = 0;
                        Image img = LoadImageAnim(m_CurrentPreviewPath.c_str(), &animFrames);
                        m_PreviewTexture = LoadTextureFromImage(img);
                        UnloadImage(img);
                    } else {
                        m_PreviewTexture = LoadTexture(m_CurrentPreviewPath.c_str());
                    }
                }
            }

            if (m_PreviewTexture.id != 0) {
                ImGui::Text(
                    "Previewing: %s",
                    std::filesystem::path(m_CurrentPreviewPath).filename().string().c_str());
                ImGui::Separator();

                ImVec2 avail = ImGui::GetContentRegionAvail();
                float aspect = (float)m_PreviewTexture.width / (float)m_PreviewTexture.height;
                ImVec2 size;
                if (avail.x / aspect <= avail.y) {
                    size.x = avail.x;
                    size.y = avail.x / aspect;
                } else {
                    size.y = avail.y;
                    size.x = avail.y * aspect;
                }

                ImVec2 cursor = ImGui::GetCursorPos();
                cursor.x += (avail.x - size.x) * 0.5f;
                cursor.y += (avail.y - size.y) * 0.5f;
                ImGui::SetCursorPos(cursor);

                rlImGuiImageSize(&m_PreviewTexture, (int)size.x, (int)size.y);
            } else {
                ImGui::Text(
                    "Previewing: %s",
                    std::filesystem::path(m_CurrentPreviewPath).filename().string().c_str());
                ImGui::TextDisabled("(TODO: support for other files)");
            }
        } else {
            ImGui::Text("No asset selected.");
        }
    }
    ImGui::End();
}

} // namespace Editor
