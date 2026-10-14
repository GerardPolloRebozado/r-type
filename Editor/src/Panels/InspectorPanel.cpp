#include "Panels/InspectorPanel.hpp"
#include "Components/Name.hpp"
#include "Core/Application.hpp"
#include "Panels/HierarchyPanel.hpp"
#include "Panels/StatePanel.hpp"
#include "Plugins/CameraPluginEditor.hpp"
#include "Plugins/ColliderPluginEditor.hpp"
#include "Plugins/PositionPluginEditor.hpp"
#include "Plugins/RotationPluginEditor.hpp"
#include "Plugins/ShapePluginEditor.hpp"
#include "Plugins/SpritePluginEditor.hpp"
#include "Plugins/TextPluginEditor.hpp"
#include "Resources/StateResource.hpp"
#include <cstring>
#include <extras/IconsFontAwesome6.h>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace Editor {
InspectorPanel::InspectorPanel() {
    m_PluginEditors.push_back(std::make_unique<PositionPluginEditor>());
    m_PluginEditors.push_back(std::make_unique<TextPluginEditor>());
    m_PluginEditors.push_back(std::make_unique<ShapePluginEditor>());
    m_PluginEditors.push_back(std::make_unique<RotationPluginEditor>());
    m_PluginEditors.push_back(std::make_unique<SpritePluginEditor>());
    m_PluginEditors.push_back(std::make_unique<ColliderPluginEditor>());
    m_PluginEditors.push_back(std::make_unique<CameraPluginEditor>());
}

void InspectorPanel::OnImGuiRender(Engine::Registry& registry, EditorConfig& config) {
    auto& selectedEntity = registry.GetResource<HierarchyPanelSelectedEntity>();
    bool hasStateSelection = false;
    StatePanelSelectedState* selectedState = nullptr;
    StateManager* stateManager = nullptr;

    if (registry.HasResource<StatePanelSelectedState>() && registry.HasResource<StateManager>()) {
        selectedState = &registry.GetResource<StatePanelSelectedState>();
        stateManager = &registry.GetResource<StateManager>();
        if (selectedState->groupIndex >= 0 &&
            selectedState->groupIndex < static_cast<int>(stateManager->groups.size())) {
            hasStateSelection = true;
        }
    }

    bool hasDirectorySelection = false;
    HierarchyPanelSelectedDirectory* selectedDir = nullptr;
    EntityDirectoriesManager* dirManager = nullptr;

    if (registry.HasResource<HierarchyPanelSelectedDirectory>() &&
        registry.HasResource<EntityDirectoriesManager>()) {
        selectedDir = &registry.GetResource<HierarchyPanelSelectedDirectory>();
        dirManager = &registry.GetResource<EntityDirectoriesManager>();
        if (selectedDir->directoryIndex >= 0 &&
            selectedDir->directoryIndex < static_cast<int>(dirManager->directories.size())) {
            hasDirectorySelection = true;
        }
    }

    if (ImGui::Begin(m_Name.c_str())) {
        if (hasStateSelection) {
            auto& group = stateManager->groups[selectedState->groupIndex];

            if (selectedState->stateIndex == -1) {
                ImGui::Text(ICON_FA_FOLDER_OPEN " State Group");
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::InputText("Group Name", &group.name);
                ImGui::Spacing();

                if (ImGui::BeginCombo("Initial State", group.initialState.empty()
                                                           ? "None"
                                                           : group.initialState.c_str())) {
                    for (const auto& st : group.states) {
                        bool isSel = (group.initialState == st.name);
                        if (ImGui::Selectable(st.name.c_str(), isSel)) {
                            group.initialState = st.name;
                        }
                    }
                    ImGui::EndCombo();
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Text("States (%zu)", group.states.size());

                static std::string newStName;
                ImGui::InputTextWithHint("##NewStateName", "New state name...", &newStName);
                ImGui::SameLine();
                if (ImGui::Button("Add State")) {
                    if (newStName.empty()) {
                        newStName = "State_" + std::to_string(group.states.size());
                    }
                    StateBody newBody;
                    newBody.name = newStName;
                    group.states.push_back(newBody);
                    if (group.initialState.empty()) {
                        group.initialState = newStName;
                    }
                    newStName.clear();
                }

                for (size_t s = 0; s < group.states.size(); ++s) {
                    ImGui::PushID(static_cast<int>(s));
                    if (ImGui::Button(ICON_FA_TRASH)) {
                        group.states.erase(group.states.begin() + s);
                        ImGui::PopID();
                        break;
                    }
                    ImGui::SameLine();
                    if (ImGui::Selectable(group.states[s].name.c_str())) {
                        selectedState->stateIndex = static_cast<int>(s);
                    }
                    ImGui::PopID();
                }
            } else if (selectedState->stateIndex >= 0 &&
                       selectedState->stateIndex < static_cast<int>(group.states.size())) {
                auto& state = group.states[selectedState->stateIndex];

                ImGui::Text(ICON_FA_TAG " State");
                ImGui::SameLine();
                ImGui::TextDisabled("(Group: %s)", group.name.c_str());
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::InputText("State Name", &state.name);
                ImGui::Spacing();

                bool isInitial = (group.initialState == state.name);
                if (isInitial) {
                    ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f),
                                       ICON_FA_CHECK " Is Initial State");
                } else {
                    if (ImGui::Button("Set as Initial State")) {
                        group.initialState = state.name;
                    }
                }

                ImGui::Spacing();
                if (registry.HasResource<Engine::NextState<std::string>>()) {
                    if (ImGui::Button(ICON_FA_PLAY " Activate Live in Engine")) {
                        registry.GetResource<Engine::NextState<std::string>>().set(state.name);
                    }
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Text("Assigned Systems (%zu)", state.systems.size());

                static std::string newSysName;
                ImGui::InputTextWithHint("##NewSysName", "System name...", &newSysName);
                ImGui::SameLine();
                if (ImGui::Button("Add System")) {
                    if (!newSysName.empty()) {
                        state.systems.push_back(newSysName);
                        newSysName.clear();
                    }
                }

                for (size_t sysIdx = 0; sysIdx < state.systems.size(); ++sysIdx) {
                    ImGui::PushID(static_cast<int>(sysIdx));
                    if (ImGui::Button(ICON_FA_TRASH)) {
                        state.systems.erase(state.systems.begin() + sysIdx);
                        ImGui::PopID();
                        break;
                    }
                    ImGui::SameLine();
                    ImGui::Text("%s", state.systems[sysIdx].c_str());
                    ImGui::PopID();
                }
            }
        } else if (hasDirectorySelection) {
            auto& dir = dirManager->directories[selectedDir->directoryIndex];
            ImGui::Text(ICON_FA_FOLDER_OPEN " Directory");
            ImGui::Separator();
            ImGui::Spacing();

            if (selectedDir->directoryIndex == 0) {
                ImGui::TextDisabled("Root Directory (/)");
            } else {
                if (ImGui::InputText("Folder Name", &dir.Name)) {
                    dir.InternalPath = "/" + dir.Name;
                }
                ImGui::TextDisabled("Internal Path: %s", dir.InternalPath.c_str());
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Text("Contained Entities (%zu)", dir.entities.size());
            for (auto ent : dir.entities) {
                ImGui::BulletText("%s (ID: %d)", Engine::GetEntityName(registry, ent).c_str(),
                                  static_cast<int>(ent));
            }
        } else if (selectedEntity.entity != -1) {
            const bool hasName = registry.HasComponent<Engine::Name>(selectedEntity.entity);
            std::string name =
                hasName ? registry.GetComponent<Engine::Name>(selectedEntity.entity).value : "";
            const std::string hint = "Entity " + std::to_string(selectedEntity.entity);
            ImGui::PushID(static_cast<int>(selectedEntity.entity));
            if (ImGui::InputTextWithHint("Name", hint.c_str(), &name)) {
                if (name.empty()) {
                    registry.RemoveComponent<Engine::Name>(selectedEntity.entity);
                } else if (hasName) {
                    registry.GetComponent<Engine::Name>(selectedEntity.entity).value = name;
                } else {
                    registry.AddComponent<Engine::Name>(selectedEntity.entity, Engine::Name(name));
                }
            }
            ImGui::PopID();
            ImGui::TextDisabled("ID: %d", static_cast<int>(selectedEntity.entity));
            ImGui::Separator();

            for (auto& editor : m_PluginEditors) {
                editor->OnImGuiRender(selectedEntity.entity, registry);
            }
        } else {
            ImGui::TextDisabled("No entity, folder, or state selected");
        }
    }

    if (ImGui::BeginPopupContextWindow("InspectorContextMenu",
                                       ImGuiPopupFlags_MouseButtonRight |
                                           ImGuiPopupFlags_NoOpenOverItems)) {
        if (selectedEntity.entity != -1 && !hasStateSelection && !hasDirectorySelection) {
            bool anyAvailable = false;
            for (auto& editor : m_PluginEditors) {
                if (!editor->HasComponent(selectedEntity.entity, registry)) {
                    anyAvailable = true;
                    std::string menuItem =
                        std::string(editor->GetIcon()) + " Add " + editor->GetName();
                    if (ImGui::MenuItem(menuItem.c_str())) {
                        editor->AddComponent(selectedEntity.entity, registry);
                    }
                }
            }
            if (!anyAvailable) {
                ImGui::TextDisabled("All plugins added");
            }
        } else {
            ImGui::TextDisabled("No entity selected");
        }
        ImGui::EndPopup();
    }
    ImGui::End();
}

const std::string& InspectorPanel::GetName() const {
    return m_Name;
}

void InspectorPanel::SetSelectedEntity(Engine::Entity entity) {
    m_SelectedEntity = entity;
}
} // namespace Editor
