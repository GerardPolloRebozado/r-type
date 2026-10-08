#include "Gallery.hpp"
#include "BundledAssets.hpp"
#include "EditorLayer.hpp"
#include "imgui.h"
#include "nlohmann/json.hpp"
#include <algorithm>
#include <extras/FA6FreeSolidFontData.h>
#include <extras/IconsFontAwesome6.h>
#include <filesystem>
#include <fstream>
#include <vector>

ImFont* g_IconFont = nullptr;

void LoadGalleryFonts() {
    ImGuiIO& io = ImGui::GetIO();

    ImFontConfig regularConfig{};
    regularConfig.FontDataOwnedByAtlas = false;

    io.Fonts->AddFontFromMemoryTTF(
        const_cast<unsigned char*>(BundledAssets::fonts_Inter_Regular_ttf),
        BundledAssets::fonts_Inter_Regular_ttf_size, 16.0f, &regularConfig);

    static const ImWchar iconRanges[] = {ICON_MIN_FA, ICON_MAX_16_FA, 0};

    ImFontConfig mergedIconConfig{};
    mergedIconConfig.MergeMode = true;
    mergedIconConfig.PixelSnapH = true;
    mergedIconConfig.FontDataOwnedByAtlas = false;

    io.Fonts->AddFontFromMemoryCompressedTTF(fa_solid_900_compressed_data,
                                             fa_solid_900_compressed_size, 16.0f, &mergedIconConfig,
                                             iconRanges);

    ImFontConfig largeIconConfig{};
    largeIconConfig.MergeMode = false;
    largeIconConfig.PixelSnapH = true;
    largeIconConfig.FontDataOwnedByAtlas = false;

    g_IconFont = io.Fonts->AddFontFromMemoryCompressedTTF(fa_solid_900_compressed_data,
                                                          fa_solid_900_compressed_size, 48.0f,
                                                          &largeIconConfig, iconRanges);
}

struct ProjectData {
    std::string name;
    std::string description;
    std::string lastOpened;
    std::string path;
    std::string engineVersion;
};

std::filesystem::path configDirectory() {
#ifdef _WIN32
    if (const char* appData = std::getenv("APPDATA")) {
        return std::filesystem::path(appData) / "rtype";
    }
    throw std::runtime_error("APPDATA is not set");

#elif defined(__APPLE__)
    if (const char* home = std::getenv("HOME")) {
        return std::filesystem::path(home) / "Library" / "Application Support" / "rtype";
    }
    throw std::runtime_error("HOME is not set");

#else // Linux and other Unix-like systems
    if (const char* xdgConfig = std::getenv("XDG_CONFIG_HOME"); xdgConfig && *xdgConfig) {
        return std::filesystem::path(xdgConfig) / "rtype";
    }

    if (const char* home = std::getenv("HOME")) {
        return std::filesystem::path(home) / ".config" / "rtype";
    }
    throw std::runtime_error("Neither XDG_CONFIG_HOME nor HOME is set");
#endif
}
namespace Editor {

std::vector<ProjectData> Gallery::FindProjects() {
    const auto settingsPath =
        configDirectory() / "projects.json"; // the / operator is overloaded to concatenate

    if (!std::filesystem::exists(settingsPath)) {
        return {};
    }

    std::ifstream file(settingsPath);
    if (!file) {
        throw std::runtime_error("Could not open " + settingsPath.string());
    }

    const nlohmann::json data = nlohmann::json::parse(file);
    if (!data.is_array()) {
        throw std::runtime_error("Expected a JSON array in " + settingsPath.string());
    }

    std::vector<ProjectData> projects;
    projects.reserve(data.size());

    for (const auto& item : data) {
        if (!item.is_object()) {
            continue;
        }

        projects.push_back({item.value("name", ""), item.value("description", ""),
                            item.value("lastOpened", ""), item.value("path", ""),
                            item.value("engineVersion", "")});
    }

    return projects;
}

void Editor::Gallery::DrawGallery() {
    static std::vector<ProjectData> projects = FindProjects();
    static char searchBuffer[128] = "";

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoSavedSettings |
                                    ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    if (ImGui::Begin("Project Gallery", nullptr, window_flags)) {

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
        ImGui::BeginChild("Sidebar", ImVec2(250, 0), false, ImGuiWindowFlags_None);

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::SetCursorPosX(20.0f);
        ImGui::SetWindowFontScale(1.5f);
        ImGui::Text("%s Engine Hub", ICON_FA_CUBES);
        ImGui::SetWindowFontScale(1.0f);
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.1f, 0.5f));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 15));

        static int selectedMenu = 0;
        if (ImGui::Selectable(ICON_FA_LIST "   Projects", selectedMenu == 0, 0, ImVec2(0, 40))) {
            selectedMenu = 0;
        }

        ImGui::PopStyleVar(2);

        ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 60);
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::SetCursorPosX(20.0f);
        if (ImGui::Button(ICON_FA_GEAR "  Settings", ImVec2(210, 30))) {
        }

        ImGui::EndChild();
        ImGui::PopStyleColor();

        ImGui::SameLine(0, 0);

        ImGui::BeginChild("MainContent", ImVec2(0, 0), false, ImGuiWindowFlags_None);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(40.0f, 40.0f));
        ImGui::BeginChild("InnerContent", ImVec2(0, 0), false, 0);

        ImGui::Dummy(ImVec2(0.0f, 15.0f));

        if (selectedMenu == 0) {
            ImGui::SetWindowFontScale(1.5f);
            ImGui::Text("Projects");
            ImGui::SetWindowFontScale(1.0f);

            float searchBarWidth = 250.0f;
            float buttonsWidth = 200.0f;
            ImGui::SameLine(ImGui::GetWindowWidth() - searchBarWidth - buttonsWidth - 20);

            ImGui::SetNextItemWidth(searchBarWidth);
            ImGui::InputTextWithHint("##Search", ICON_FA_MAGNIFYING_GLASS " Search...",
                                     searchBuffer, IM_ARRAYSIZE(searchBuffer));

            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_PLUS " New Project", ImVec2(120, 0))) {
            }

            ImGui::SameLine();
            if (ImGui::Button("Open", ImVec2(60, 0))) {
            }

            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::Spacing();

            // Projects List
            for (size_t i = 0; i < projects.size(); ++i) {
                std::string searchStr(searchBuffer);
                std::string nameLower = projects[i].name;
                std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
                std::string searchLower = searchStr;
                std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(),
                               ::tolower);

                if (!searchLower.empty() && nameLower.find(searchLower) == std::string::npos) {
                    continue;
                }

                ImGui::PushID(static_cast<int>(i));
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

                if (ImGui::BeginChild("ProjectItem", ImVec2(0, 90), true,
                                      ImGuiWindowFlags_NoScrollbar |
                                          ImGuiWindowFlags_NoScrollWithMouse)) {

                    ImVec2 p = ImGui::GetCursorScreenPos();

                    if (ImGui::InvisibleButton("##click", ImGui::GetContentRegionAvail())) {
                        // Logic when a project is clicked
                    }

                    if (ImGui::IsItemHovered()) {
                        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                        ImVec4 hoverColor = ImGui::GetStyle().Colors[ImGuiCol_HeaderHovered];
                        hoverColor.w = 0.3f;
                        ImGui::GetWindowDrawList()->AddRectFilled(
                            ImGui::GetWindowPos(),
                            ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowWidth(),
                                   ImGui::GetWindowPos().y + ImGui::GetWindowHeight()),
                            ImGui::GetColorU32(hoverColor));
                    }

                    ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(p.x + 15, p.y + 15),
                                                              ImVec2(p.x + 15 + 60, p.y + 15 + 60),
                                                              IM_COL32(80, 80, 80, 255), 8.0f);

                    ImVec2 iconSize = ImGui::CalcTextSize(ICON_FA_GAMEPAD);
                    float iconX = (60.0f - iconSize.x) * 0.5f;
                    float iconY = (60.0f - iconSize.y) * 0.5f;

                    ImGui::SetCursorScreenPos(ImVec2(p.x + 15 + iconX, p.y + 15 + iconY));
                    ImGui::TextColored(ImVec4(1, 1, 1, 0.5f), ICON_FA_GAMEPAD);

                    ImGui::SetCursorScreenPos(ImVec2(p.x + 90, p.y + 20));

                    ImGui::BeginGroup();
                    ImGui::SetWindowFontScale(1.1f);
                    ImGui::TextUnformatted(projects[i].name.c_str());
                    ImGui::SetWindowFontScale(1.0f);

                    ImGui::TextDisabled("%s", projects[i].path.c_str());
                    ImGui::EndGroup();

                    std::string lastOpenedStr = projects[i].lastOpened;
                    float rightAlign = ImGui::GetWindowWidth() - 250;

                    ImGui::SetCursorScreenPos(ImVec2(p.x + rightAlign, p.y + 25));
                    ImGui::TextDisabled("Engine: %s", projects[i].engineVersion.c_str());

                    ImGui::SetCursorScreenPos(ImVec2(p.x + rightAlign, p.y + 45));
                    ImGui::TextDisabled("Opened: %s", lastOpenedStr.c_str());

                    ImGui::SetCursorScreenPos(ImVec2(p.x + ImGui::GetWindowWidth() - 50, p.y + 25));

                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 1, 0.1f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1, 1, 1, 0.2f));
                    if (ImGui::Button(ICON_FA_ELLIPSIS_VERTICAL "##opts", ImVec2(30, 30))) {
                        ImGui::OpenPopup("ProjectOptions");
                    }
                    ImGui::PopStyleColor(3);

                    if (ImGui::BeginPopup("ProjectOptions")) {
                        if (ImGui::Selectable(ICON_FA_FOLDER_OPEN " Show in Explorer")) {
                        }
                        if (ImGui::Selectable(ICON_FA_PEN " Rename")) {
                        }
                        ImGui::Separator();
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
                        if (ImGui::Selectable(ICON_FA_TRASH " Remove from list")) {
                        }
                        ImGui::PopStyleColor();
                        ImGui::EndPopup();
                    }
                }
                ImGui::EndChild();
                ImGui::PopStyleVar(2);

                ImGui::PopID();
                ImGui::Spacing();
            }
        } else {
            ImGui::Spacing();
            ImGui::TextDisabled("This section is currently empty.");
        }

        ImGui::EndChild();
        ImGui::PopStyleVar();

        ImGui::EndChild();
    }
    ImGui::End();
    ImGui::PopStyleVar();
}
} // namespace Editor
