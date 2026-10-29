/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** AnimationPluginEditor.cpp
*/
#include "../../include/Plugins/AnimationPluginEditor.hpp"

#include <iostream>

#include "../../../.cpm_cache/imgui/095be9d03563deb24f9ec06d03c806c7f0ada846/imgui.h"
#include "../../../cmake-build-debug/CPM_Cache/rlimgui/7253b7b7a0bf1636d7899c7500d4ef4d5018c37d/extras/IconsFontAwesome6.h"
#include "Plugins/AnimationPlugin.hpp"

namespace Editor {
bool AnimationPluginEditor::OnImGuiRender(Engine::Entity entity, Engine::Registry& registry) {
    if (!registry.HasComponent<Engine::Animation>(entity)) {
        return false;
    }

    auto& animation = registry.GetComponent<Engine::Animation>(entity);
    auto isOpen = ImGui::CollapsingHeader((std::string(GetIcon()) + " " + GetName()).c_str(),
                                          ImGuiTreeNodeFlags_DefaultOpen);

    if (isOpen) {
        for (auto name : animation.names) {
            ImGui::Text("Animation: %s", name.c_str());
        }
    }
    return true;
}

const char* AnimationPluginEditor::GetIcon() const {
    return ICON_FA_FILM;
}

const char* AnimationPluginEditor::GetName() const {
    return "Animation";
}

bool AnimationPluginEditor::HasComponent(Engine::Entity entity, Engine::Registry& registry) {
    return registry.HasComponent<Engine::Animation>(entity);
}

void AnimationPluginEditor::AddComponent(Engine::Entity entity, Engine::Registry& registry) {
    Engine::AnimationPlugin::AttachToEntity(registry, entity, {});
}

void AnimationPluginEditor::RemoveComponent(Engine::Entity entity, Engine::Registry& registry) {
    if (!registry.HasComponent<Engine::Animation>(entity)) {
        return;
    }
    registry.RemoveComponent<Engine::Animation>(entity);
}
} // namespace Editor