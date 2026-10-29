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

#include "FileDialog.hpp"

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
            ImGui::SetNextWindowSize(ImVec2(850, 750), ImGuiCond_Once);

            if (ImGui::Begin("Animation Editor", &showAnimatorWindow)) {

                ImGui::TextDisabled("Preview");

                ImVec2 viewportSize(ImGui::GetContentRegionAvail().x, 220.0f);
                ImGui::BeginChild("AnimationPreviewViewport", viewportSize, true,
                                  ImGuiWindowFlags_NoScrollbar);

                if (!animation.sprites.empty() &&
                    animation.currentFrame < animation.sprites.size() &&
                    animation.sprites[animation.currentFrame]) {

                    auto& texture = animation.sprites[animation.currentFrame]->GetTexture();
                    auto textureId = static_cast<ImTextureID>(texture.id);
                    float spriteW = (animation.frameWidth > 0.0f)
                                        ? animation.frameWidth
                                        : static_cast<float>(texture.width);
                    float spriteH = (animation.frameHeight > 0.0f)
                                        ? animation.frameHeight
                                        : static_cast<float>(texture.height);

                    float scale = std::min((viewportSize.x - 20.0f) / spriteW,
                                           (viewportSize.y - 20.0f) / spriteH);
                    ImVec2 displaySize(spriteW * scale, spriteH * scale);

                    ImGui::SetCursorPos(ImVec2((viewportSize.x - displaySize.x) * 0.5f,
                                               (viewportSize.y - displaySize.y) * 0.5f));

                    ImVec2 uv0(0.0f, 0.0f);
                    ImVec2 uv1(1.0f, 1.0f);

                    if (animation.frameWidth > 0.0f && animation.frameHeight > 0.0f) {
                        float texW = static_cast<float>(texture.width);
                        float texH = static_cast<float>(texture.height);

                        int cols = static_cast<int>(texW / animation.frameWidth);
                        if (cols > 0) {
                            int col = static_cast<int>(animation.currentFrame) % cols;
                            int row = static_cast<int>(animation.currentFrame) / cols;

                            uv0 = ImVec2((col * animation.frameWidth) / texW,
                                         (row * animation.frameHeight) / texH);
                            uv1 = ImVec2(((col + 1) * animation.frameWidth) / texW,
                                         ((row + 1) * animation.frameHeight) / texH);
                        }
                    }

                    if (animation.flipX) {
                        std::swap(uv0.x, uv1.x);
                    }
                    if (animation.flipY) {
                        std::swap(uv0.y, uv1.y);
                    }

                    ImGui::Image(textureId, displaySize, uv0, uv1);
                }

                ImGui::EndChild();

                if (ImGui::Button(animation.isPlaying ? "Pause" : "Play")) {
                    animation.isPlaying = !animation.isPlaying;
                }
                ImGui::SameLine();
                ImGui::Checkbox("Loop", &animation.loop);

                ImGui::Separator();

                ImGui::TextDisabled("FRAMES / SPRITES of the Animation");

                ImGui::BeginChild("SpritesGallery", ImVec2(0, 140.0f), true,
                                  ImGuiWindowFlags_HorizontalScrollbar);

                size_t totalFrameCount =
                    !animation.frameData.empty()
                        ? animation.frameData.size()
                        : (animation.totalFrames > 0 ? animation.totalFrames
                                                     : animation.sprites.size());

                for (size_t i = 0; i < totalFrameCount; ++i) {
                    ImGui::PushID(static_cast<int>(i));

                    bool isCurrent = (i == animation.currentFrame);
                    if (isCurrent) {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.9f, 0.6f));
                    }

                    std::string frameLabel = "Frame " + std::to_string(i);
                    if (ImGui::Button(frameLabel.c_str(), ImVec2(90.0f, 90.0f))) {
                        animation.currentFrame = i;
                    }

                    if (isCurrent) {
                        ImGui::PopStyleColor();
                    }

                    ImGui::SameLine();
                    ImGui::PopID();
                }

                ImGui::EndChild();

                ImGui::Separator();

                ImGui::TextDisabled("Parameters");

                static int selectedAnimIndex = 0;
                std::string currentAnimName =
                    (selectedAnimIndex < static_cast<int>(animation.names.size()))
                        ? animation.names[selectedAnimIndex]
                        : "None";

                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 180.0f);
                if (ImGui::BeginCombo("Active Animation", currentAnimName.c_str())) {
                    for (int i = 0; i < static_cast<int>(animation.names.size()); ++i) {
                        const bool isSelected = (selectedAnimIndex == i);
                        if (ImGui::Selectable(animation.names[i].c_str(), isSelected)) {
                            selectedAnimIndex = i;
                        }
                        if (isSelected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }

                ImGui::SameLine();

                if (ImGui::Button("Search", ImVec2(160.0f, 0))) {
                    std::vector<std::string> selectedFiles =
                        Editor::PickFilesDialog("Select Sprites");

                    if (!selectedFiles.empty()) {
                        animation.assetPaths.insert(animation.assetPaths.end(),
                                                    selectedFiles.begin(), selectedFiles.end());

                        animation.sprites.clear();
                        animation.currentFrame = 0;
                        animation.timer = 0.0f;
                    }
                }

                if (!animation.assetPaths.empty()) {
                    ImGui::TextWrapped("File: %s", animation.assetPaths[0].c_str());
                }

                ImGui::Separator();

                int currentFrame = static_cast<int>(animation.currentFrame);
                if (ImGui::InputInt("Actual Frame", &currentFrame)) {
                    if (currentFrame >= 0 && static_cast<size_t>(currentFrame) < totalFrameCount) {
                        animation.currentFrame = static_cast<size_t>(currentFrame);
                    }
                }

                int totalFrames = static_cast<int>(animation.totalFrames);
                if (ImGui::InputInt("Total Frames", &totalFrames)) {
                    if (totalFrames >= 0) {
                        animation.totalFrames = static_cast<size_t>(totalFrames);
                    }
                }

                ImGui::InputFloat("Frame Width", &animation.frameWidth);
                ImGui::InputFloat("Frame Height", &animation.frameHeight);
                ImGui::InputFloat("Frame Duration", &animation.frameDuration);
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