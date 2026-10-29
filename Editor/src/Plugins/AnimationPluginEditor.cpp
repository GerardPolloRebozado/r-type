/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** AnimationPluginEditor.cpp
*/
#include "../../include/Plugins/AnimationPluginEditor.hpp"

#include "Plugins/AnimationPlugin.hpp"
#include "misc/cpp/imgui_stdlib.h"
#include <extras/IconsFontAwesome6.h>
#include <imgui.h>
#include <iostream>

namespace Editor {
bool AnimationPluginEditor::OnImGuiRender(Engine::Entity entity, Engine::Registry& registry) {
    if (!registry.HasComponent<Engine::Animation>(entity)) {
        return false;
    }

    auto& animation = registry.GetComponent<Engine::Animation>(entity);
    auto isOpen = ImGui::CollapsingHeader((std::string(GetIcon()) + " " + GetName()).c_str(),
                                          ImGuiTreeNodeFlags_DefaultOpen);

    if (isOpen) {
        for (auto& name : animation.names) {
            ImGui::Text("Animation: %s", name.c_str());
        }
        ImGui::Separator();
        static std::string newAnimBuffer;

        ImGui::SetNextItemWidth(-40.0f);

        bool isEnterPressed =
            ImGui::InputText("##NewAnim", &newAnimBuffer, ImGuiInputTextFlags_EnterReturnsTrue);

        ImGui::SameLine();
        bool isButtonPressed = ImGui::Button("+", ImVec2(32.0f, 0));

        if ((isEnterPressed || isButtonPressed) && !newAnimBuffer.empty()) {
            animation.names.push_back(newAnimBuffer);
            newAnimBuffer.clear();
        }

        ImGui::Separator();

        static bool showAnimatorWindow = false;
        if (ImGui::Button("Animator")) {
            showAnimatorWindow = true;
        }

        if (showAnimatorWindow) {
            ImGui::SetNextWindowSize(ImVec2(800, 700), ImGuiCond_Once);

            if (ImGui::Begin("Animator", &showAnimatorWindow)) {

                int currentFrame = static_cast<int>(animation.currentFrame);
                if (ImGui::InputInt("Current Frame", &currentFrame)) {
                    if (currentFrame >= 0) {
                        animation.currentFrame = static_cast<std::size_t>(currentFrame);
                    }
                }

                int totalFrames = static_cast<int>(animation.totalFrames);
                if (ImGui::InputInt("Total Frames", &totalFrames)) {
                    if (totalFrames >= 0) {
                        animation.totalFrames = static_cast<std::size_t>(totalFrames);
                    }
                }

                ImGui::InputFloat("Frame Width", &animation.frameWidth);
                ImGui::InputFloat("Frame Height", &animation.frameHeight);

                ImGui::Separator();

                ImGui::InputFloat("Frame Duration", &animation.frameDuration);
                ImGui::InputFloat("Timer", &animation.timer);

                ImGui::Separator();

                ImGui::InputFloat("Offset X", &animation.offsetX);
                ImGui::InputFloat("Offset Y", &animation.offsetY);
                ImGui::InputFloat("Scale X", &animation.scaleX);
                ImGui::InputFloat("Scale Y", &animation.scaleY);

                ImGui::Separator();

                ImGui::Checkbox("Loop", &animation.loop);
                ImGui::Checkbox("Is Playing", &animation.isPlaying);
                ImGui::Checkbox("Flip X", &animation.flipX);
                ImGui::Checkbox("Flip Y", &animation.flipY);

                ImGui::Separator();

                ImGui::Button("Animate");
            }
            ImGui::End();
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