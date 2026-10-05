#include "Panels/AssetsPanel.hpp"
#include "EditorConfig.hpp"
#include "EditorLayer.hpp"
#include <extras/IconsFontAwesome6.h>
#include <imgui.h>

namespace Editor {

void AssetsPanel::OnImGuiRender(Engine::Registry& registry, EditorConfig& cfg) {
    if (!ImGui::Begin(m_Name.c_str())) {
        ImGui::End();
        return;
    }

    if (cfg.m_ProjectFolder.empty()) {
        ImGui::Text("No project loaded.");
        ImGui::End();
        return;
    }

    std::error_code error;

    std::filesystem::path projectPath =
        std::filesystem::weakly_canonical(std::filesystem::path(cfg.m_ProjectFolder), error);

    if (error || !std::filesystem::exists(projectPath) ||
        !std::filesystem::is_directory(projectPath)) {
        ImGui::Text("Invalid project folder.");
        ImGui::End();
        return;
    }

    /*
     * Reset the navigation root when a different project is loaded.
     *
     * m_StartingDirectory is the highest directory the user is allowed
     * to navigate back to.
     */
    if (m_LastProjectDirectory != projectPath) {
        m_LastProjectDirectory = projectPath;
        m_StartingDirectory = projectPath;
        m_CurrentDirectory = projectPath;
    }

    if (m_StartingDirectory.empty()) {
        m_StartingDirectory = projectPath;
    }

    if (m_CurrentDirectory.empty() || !std::filesystem::exists(m_CurrentDirectory) ||
        !std::filesystem::is_directory(m_CurrentDirectory)) {
        m_CurrentDirectory = m_StartingDirectory;
    }

    /*
     * Right-click context menu for the current folder.
     */
    if (ImGui::BeginPopupContextWindow("AssetsFolderContext", ImGuiPopupFlags_MouseButtonRight)) {
        if (ImGui::Button("Refresh Folder")) {
            /*
             * The folder contents are enumerated directly below every
             * frame, so normalizing the path is enough to refresh it.
             */
            if (std::filesystem::exists(m_CurrentDirectory) &&
                std::filesystem::is_directory(m_CurrentDirectory)) {
                m_CurrentDirectory = std::filesystem::weakly_canonical(m_CurrentDirectory, error);

                if (error) {
                    m_CurrentDirectory = m_StartingDirectory;
                    error.clear();
                }
            } else {
                m_CurrentDirectory = m_StartingDirectory;
            }

            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    /*
     * Only allow going back if we are below the starting directory.
     */
    const bool canGoBack = !m_CurrentDirectory.empty() && m_CurrentDirectory != m_StartingDirectory;

    if (canGoBack) {
        if (ImGui::Button("<- Back")) {
            const std::filesystem::path parentDirectory = m_CurrentDirectory.parent_path();

            /*
             * Prevent invalid navigation at the filesystem root.
             * Because folders are only entered from this function,
             * this will never move above m_StartingDirectory.
             */
            if (!parentDirectory.empty() && parentDirectory != m_CurrentDirectory) {
                m_CurrentDirectory = parentDirectory;
            }
        }

        ImGui::SameLine();
    }

    ImGui::Text("%s", m_CurrentDirectory.string().c_str());

    ImGui::Separator();

    constexpr float padding = 16.0f;
    constexpr float thumbnailSize = 64.0f;
    constexpr float cellSize = thumbnailSize + padding;

    const float panelWidth = ImGui::GetContentRegionAvail().x;

    int columnCount = static_cast<int>(panelWidth / cellSize);

    if (columnCount < 1) {
        columnCount = 1;
    }

    if (ImGui::BeginTable("AssetsTable", columnCount)) {
        std::filesystem::directory_iterator directoryIterator(m_CurrentDirectory, error);

        if (!error) {
            for (const auto& directoryEntry : directoryIterator) {
                const std::filesystem::path path = directoryEntry.path();

                const std::string filenameString = path.filename().string();

                ImGui::TableNextColumn();
                ImGui::PushID(filenameString.c_str());

                if (directoryEntry.is_directory()) {
                    ImGui::PushFont(g_IconFont);
                    bool clicked =
                        ImGui::Button(ICON_FA_FOLDER, ImVec2(thumbnailSize, thumbnailSize));
                    ImGui::PopFont();

                    if (clicked) {
                        /*
                         * Enter the folder and normalize its path.
                         */
                        m_CurrentDirectory = std::filesystem::weakly_canonical(path, error);

                        if (error) {
                            m_CurrentDirectory = m_StartingDirectory;
                            error.clear();
                        }
                    }
                } else {
                    ImGui::PushFont(g_IconFont);
                    if (ImGui::Button(ICON_FA_FILE, ImVec2(thumbnailSize, thumbnailSize))) {
                        cfg.m_SelectedAssetPath = path.string();
                    }
                    ImGui::PopFont();
                }

                ImGui::TextWrapped("%s", filenameString.c_str());

                ImGui::PopID();
            }
        }

        ImGui::EndTable();
    }

    ImGui::End();
}

} // namespace Editor

namespace Editor {
const std::string& AssetsPanel::GetName() const {
    return m_Name;
}
} // namespace Editor
