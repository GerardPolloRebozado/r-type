#include "Panels/WavePanel.hpp"
#include "EditorConfig.hpp"
#include "Resources/WaveSchedule.hpp"
#include <algorithm>
#include <filesystem>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <string>
#include <vector>

namespace Editor {
namespace {

constexpr const char* kFormationNames[] = {"Line", "V Formation", "Sine", "Grid"};
constexpr int kFormationCount = 4;

constexpr const char* kGroupFormationScript = "scripts/group-formation.lua";
constexpr const char* kGroupSnakeScript = "scripts/group-patapata.lua";

const char* FormationLabel(Engine::WavePatternType formation) {
    const int index = static_cast<int>(formation);
    if (index < 0 || index >= kFormationCount) {
        return kFormationNames[0];
    }
    return kFormationNames[index];
}

Engine::WaveDefinition MakeDefaultWave(const std::vector<Engine::WaveDefinition>& waves) {
    Engine::WaveDefinition wave;
    wave.name = "Wave " + std::to_string(waves.size() + 1);
    wave.prefabPath = "Prefabs/Pata-Pata.prefab";
    wave.formation = Engine::WavePatternType::Line;
    wave.count = 5;
    wave.spacing = 100.0f;
    wave.baseY = -120.0f;
    wave.spawnOffsetX = 900.0f;

    if (!waves.empty()) {
        const Engine::WaveDefinition& previous = waves.back();
        wave.triggerX = previous.triggerX + 300.0f;
        wave.prefabPath = previous.prefabPath;
        wave.formation = previous.formation;
        wave.count = previous.count;
        wave.spacing = previous.spacing;
        wave.baseY = previous.baseY;
        wave.spawnOffsetX = previous.spawnOffsetX;
        wave.attachParent = previous.attachParent;
        wave.groupScriptPath = previous.groupScriptPath;
    }
    return wave;
}

std::vector<std::string> ListPrefabs(const std::string& projectFolder) {
    std::vector<std::string> prefabs;
    const std::filesystem::path roots[] = {std::filesystem::path(projectFolder) / "Prefabs",
                                           std::filesystem::path("Prefabs")};
    for (const auto& root : roots) {
        std::error_code ec;
        if (!std::filesystem::is_directory(root, ec)) {
            continue;
        }
        for (std::filesystem::recursive_directory_iterator it(root, ec), end; !ec && it != end;
             it.increment(ec)) {
            if (!it->is_regular_file(ec) || it->path().extension() != ".prefab") {
                continue;
            }
            std::string relative =
                std::filesystem::relative(it->path(), projectFolder, ec).generic_string();
            if (ec || relative.empty()) {
                relative = it->path().generic_string();
            }
            if (relative.rfind("./", 0) == 0) {
                relative.erase(0, 2);
            }
            if (std::find(prefabs.begin(), prefabs.end(), relative) == prefabs.end()) {
                prefabs.push_back(std::move(relative));
            }
        }
    }
    std::sort(prefabs.begin(), prefabs.end());
    return prefabs;
}

} // namespace

bool WavePanel::s_IsOpen = false;

WavePanel::WavePanel() : m_Name(ICON_FA_GHOST " Waves") {}

const std::string& WavePanel::GetName() const {
    return m_Name;
}

void WavePanel::OnImGuiRender(Engine::Registry& registry, EditorConfig& config) {
    (void)registry;
    if (!s_IsOpen) {
        return;
    }

    if (ImGui::Begin(m_Name.c_str(), &s_IsOpen)) {
        ImGui::TextDisabled("Waves spawn when the active camera reaches Activation X.");
        ImGui::TextDisabled("Waves with the same Activation X spawn together.");

        if (ImGui::Button(ICON_FA_PLUS " Add Wave")) {
            config.m_Waves.push_back(MakeDefaultWave(config.m_Waves));
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_TRASH " Clear All")) {
            config.m_Waves.clear();
        }

        ImGui::Separator();

        const std::vector<std::string> prefabs = ListPrefabs(config.m_ProjectFolder);

        int waveToDelete = -1;
        for (std::size_t i = 0; i < config.m_Waves.size(); ++i) {
            ImGui::PushID(static_cast<int>(i));
            Engine::WaveDefinition& wave = config.m_Waves[i];

            const std::string title =
                (wave.name.empty() ? "Wave " + std::to_string(i + 1) : wave.name) + "  (x " +
                std::to_string(static_cast<int>(wave.triggerX)) + ", " +
                FormationLabel(wave.formation) + " x" + std::to_string(wave.count) + ")";

            const bool open =
                ImGui::TreeNodeEx("##wave", ImGuiTreeNodeFlags_DefaultOpen, "%s", title.c_str());
            ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 28.0f);
            if (ImGui::Button(ICON_FA_TRASH)) {
                waveToDelete = static_cast<int>(i);
            }

            if (open) {
                ImGui::InputText("Name", &wave.name);
                ImGui::DragFloat("Activation X", &wave.triggerX, 10.0f, -100000.0f, 100000.0f,
                                 "%.0f");

                ImGui::InputTextWithHint("Prefab", "Prefabs/MyEnemy.prefab", &wave.prefabPath);
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_FOLDER_OPEN "##prefabPicker")) {
                    ImGui::OpenPopup("##prefabPresets");
                }
                if (ImGui::BeginPopup("##prefabPresets")) {
                    if (prefabs.empty()) {
                        ImGui::TextDisabled("No .prefab files found");
                    }
                    for (const std::string& prefab : prefabs) {
                        if (ImGui::Selectable(prefab.c_str(), wave.prefabPath == prefab)) {
                            wave.prefabPath = prefab;
                        }
                    }
                    ImGui::EndPopup();
                }

                int formation = static_cast<int>(wave.formation);
                if (formation < 0 || formation >= kFormationCount) {
                    formation = 0;
                }
                if (ImGui::Combo("Formation", &formation, kFormationNames, kFormationCount)) {
                    wave.formation = static_cast<Engine::WavePatternType>(formation);
                }

                int count = static_cast<int>(wave.count);
                if (ImGui::DragInt("Count", &count, 1, 1, 100)) {
                    wave.count = static_cast<unsigned int>(count < 1 ? 1 : count);
                }

                if (ImGui::TreeNode("Advanced")) {
                    ImGui::DragFloat("Spacing", &wave.spacing, 1.0f, 1.0f, 2000.0f, "%.0f");
                    ImGui::DragFloat("Base Y", &wave.baseY, 1.0f, -20000.0f, 20000.0f, "%.0f");
                    ImGui::DragFloat("Spawn Offset X", &wave.spawnOffsetX, 10.0f, 0.0f, 20000.0f,
                                     "%.0f");
                    ImGui::Checkbox("Attach To Group", &wave.attachParent);
                    if (wave.attachParent) {
                        if (wave.groupScriptPath.empty()) {
                            wave.groupScriptPath = kGroupFormationScript;
                        }
                        const char* modes[] = {"Formation (keep pattern)", "Snake (trail)"};
                        int mode = wave.groupScriptPath == kGroupSnakeScript ? 1 : 0;
                        if (ImGui::Combo("Group Behavior", &mode, modes, 2)) {
                            wave.groupScriptPath =
                                mode == 1 ? kGroupSnakeScript : kGroupFormationScript;
                        }
                        ImGui::InputTextWithHint("##groupScript", "scripts/group-formation.lua",
                                                 &wave.groupScriptPath);
                    }
                    ImGui::TreePop();
                }

                ImGui::TreePop();
            }

            ImGui::PopID();
            ImGui::Separator();
        }

        if (waveToDelete >= 0 && waveToDelete < static_cast<int>(config.m_Waves.size())) {
            config.m_Waves.erase(config.m_Waves.begin() + waveToDelete);
        }
    }
    ImGui::End();
}

} // namespace Editor
