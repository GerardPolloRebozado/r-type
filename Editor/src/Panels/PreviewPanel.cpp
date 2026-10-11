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

void PreviewPanel::OnImGuiRender(Engine::Registry& registry, EditorConfig& config) {
    if (!config.m_SelectedAssetPath.empty()) {
        m_IsOpen = true;
    }

    if (!m_IsOpen) {
        return;
    }

    if (ImGui::Begin(m_Name.c_str())) {
        if (!config.m_SelectedAssetPath.empty()) {
            std::string selectedPath =
                ResolveAssetPath(registry, config.m_SelectedAssetPath).string();

            if (m_CurrentPreviewPath != selectedPath) {
                m_CurrentPreviewPath = selectedPath;

                if (IsImageFile(m_CurrentPreviewPath)) {
                    std::string ext =
                        std::filesystem::path(m_CurrentPreviewPath).extension().string();
                    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

                    try {
                        if (m_PreviewTexture.IsValid()) {
                            m_PreviewTexture.Unload();
                        }
                        if (ext == ".gif") {
                            int animFrames = 0;
                            raylib::Image img;
                            img.Load(m_CurrentPreviewPath, &animFrames);
                            m_PreviewTexture.Load(img);
                        } else {
                            m_PreviewTexture.Load(m_CurrentPreviewPath);
                        }
                    } catch (const raylib::RaylibException&) {
                        m_PreviewTexture.Unload();
                    }
                } else {
                    if (m_PreviewTexture.IsValid()) {
                        m_PreviewTexture.Unload();
                    }
                }
            }

            if (m_PreviewTexture.IsValid()) {
                ImGui::Text(
                    "Previewing: %s",
                    std::filesystem::path(m_CurrentPreviewPath).filename().string().c_str());
                ImGui::Separator();

                ImVec2 avail = ImGui::GetContentRegionAvail();
                float aspect = static_cast<float>(m_PreviewTexture.width) /
                               static_cast<float>(m_PreviewTexture.height);
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

                rlImGuiImageSize(&m_PreviewTexture, static_cast<int>(size.x),
                                 static_cast<int>(size.y));
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

namespace Editor {
const std::string& PreviewPanel::GetName() const {
    return m_Name;
}
} // namespace Editor
