/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** StatePanel.cpp
*/
#include "Panels/StatePanel.hpp"
#include "Panels/HierarchyPanel.hpp"
#include <imgui.h>

namespace Editor {

void StatePanel::OnImGuiRender(Engine::Registry& registry, EditorConfig& config) {
    if (!registry.HasResource<StatePanelSelectedState>()) {
        registry.AddResource<StatePanelSelectedState>();
    }
    if (!registry.HasResource<StateManager>()) {
        registry.AddResource<StateManager>();
    }

    auto& selected = registry.GetResource<StatePanelSelectedState>();
    auto& stateManager = registry.GetResource<StateManager>();

    if (ImGui::Begin(m_Name.c_str())) {
        if (ImGui::BeginPopupContextWindow("StatePanelContextMenu",
                                           ImGuiPopupFlags_MouseButtonRight |
                                               ImGuiPopupFlags_NoOpenOverItems)) {
            if (ImGui::MenuItem("Create State Group")) {
                StateGroup newGroup;
                newGroup.name = "StateGroup_" + std::to_string(stateManager.groups.size());
                stateManager.groups.push_back(newGroup);

                selected.groupIndex = static_cast<int>(stateManager.groups.size() - 1);
                selected.stateIndex = -1;

                if (registry.HasResource<HierarchyPanelSelectedEntity>()) {
                    registry.GetResource<HierarchyPanelSelectedEntity>().entity = -1;
                }
                if (registry.HasResource<HierarchyPanelSelectedDirectory>()) {
                    registry.GetResource<HierarchyPanelSelectedDirectory>().directoryIndex = -1;
                }
            }
            ImGui::EndPopup();
        }

        if (stateManager.groups.empty()) {
            ImGui::TextDisabled("Right click to create a state group...");
        } else {
            for (size_t groupSize = 0; groupSize < stateManager.groups.size(); ++groupSize) {
                auto& group = stateManager.groups[groupSize];

                ImGui::PushID(static_cast<int>(groupSize));

                ImGuiTreeNodeFlags groupFlags =
                    ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
                if (selected.groupIndex == static_cast<int>(groupSize) &&
                    selected.stateIndex == -1) {
                    groupFlags |= ImGuiTreeNodeFlags_Selected;
                }

                bool groupOpen = ImGui::TreeNodeEx(group.name.c_str(), groupFlags);

                if (ImGui::IsItemClicked()) {
                    selected.groupIndex = static_cast<int>(groupSize);
                    selected.stateIndex = -1;

                    if (registry.HasResource<HierarchyPanelSelectedEntity>()) {
                        registry.GetResource<HierarchyPanelSelectedEntity>().entity = -1;
                    }
                    if (registry.HasResource<HierarchyPanelSelectedDirectory>()) {
                        registry.GetResource<HierarchyPanelSelectedDirectory>().directoryIndex = -1;
                    }
                }

                if (ImGui::BeginPopupContextItem()) {
                    if (ImGui::MenuItem("Add State")) {
                        StateBody newBody;
                        newBody.name = "State_" + std::to_string(group.states.size());
                        group.states.push_back(newBody);

                        selected.groupIndex = static_cast<int>(groupSize);
                        selected.stateIndex = static_cast<int>(group.states.size() - 1);
                        if (group.initialState.empty()) {
                            group.initialState = newBody.name;
                        }

                        if (registry.HasResource<HierarchyPanelSelectedEntity>()) {
                            registry.GetResource<HierarchyPanelSelectedEntity>().entity = -1;
                        }
                        if (registry.HasResource<HierarchyPanelSelectedDirectory>()) {
                            registry.GetResource<HierarchyPanelSelectedDirectory>().directoryIndex =
                                -1;
                        }
                    }
                    if (ImGui::MenuItem("Delete State Group")) {
                        stateManager.groups.erase(stateManager.groups.begin() + groupSize);
                        if (selected.groupIndex == static_cast<int>(groupSize)) {
                            selected.groupIndex = -1;
                            selected.stateIndex = -1;
                        } else if (selected.groupIndex > static_cast<int>(groupSize)) {
                            selected.groupIndex--;
                        }
                        ImGui::EndPopup();
                        if (groupOpen) {
                            ImGui::TreePop();
                        }
                        ImGui::PopID();
                        break;
                    }
                    ImGui::EndPopup();
                }

                if (groupOpen) {
                    if (group.states.empty()) {
                        ImGui::TreePush("empty_state_slot");
                        ImGui::TextDisabled("Right click group to add a state...");
                        ImGui::TreePop();
                    } else {
                        for (size_t s = 0; s < group.states.size(); ++s) {
                            const auto& body = group.states[s];
                            std::string label = body.name;

                            if (body.name == group.initialState) {
                                label += " [Initial]";
                            }

                            ImGui::PushID(static_cast<int>(s));

                            ImGuiTreeNodeFlags stateFlags =
                                ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
                            if (selected.groupIndex == static_cast<int>(groupSize) &&
                                selected.stateIndex == static_cast<int>(s)) {
                                stateFlags |= ImGuiTreeNodeFlags_Selected;
                            }

                            ImGui::TreeNodeEx(label.c_str(), stateFlags);

                            if (ImGui::IsItemClicked()) {
                                selected.groupIndex = static_cast<int>(groupSize);
                                selected.stateIndex = static_cast<int>(s);

                                if (registry.HasResource<HierarchyPanelSelectedEntity>()) {
                                    registry.GetResource<HierarchyPanelSelectedEntity>().entity =
                                        -1;
                                }
                                if (registry.HasResource<HierarchyPanelSelectedDirectory>()) {
                                    registry.GetResource<HierarchyPanelSelectedDirectory>()
                                        .directoryIndex = -1;
                                }
                            }

                            if (ImGui::BeginPopupContextItem()) {
                                if (ImGui::MenuItem("Set as Initial State")) {
                                    group.initialState = group.states[s].name;
                                }
                                if (ImGui::MenuItem("Delete State")) {
                                    group.states.erase(group.states.begin() + s);
                                    if (selected.groupIndex == static_cast<int>(groupSize) &&
                                        selected.stateIndex == static_cast<int>(s)) {
                                        selected.stateIndex = -1;
                                    } else if (selected.groupIndex == static_cast<int>(groupSize) &&
                                               selected.stateIndex > static_cast<int>(s)) {
                                        selected.stateIndex--;
                                    }
                                    ImGui::EndPopup();
                                    ImGui::PopID();
                                    break;
                                }
                                ImGui::EndPopup();
                            }

                            ImGui::PopID();
                        }
                    }
                    ImGui::TreePop();
                }

                ImGui::PopID();
            }
        }
    }
    ImGui::End();
}

const std::string& StatePanel::GetName() const {
    return m_Name;
}

} // namespace Editor