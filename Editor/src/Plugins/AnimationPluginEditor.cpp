/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** AnimationPluginEditor.cpp
*/
#include "../../include/Plugins/AnimationPluginEditor.hpp"

#include "FileDialog.hpp"
#include "Plugins/AnimationPlugin.hpp"
#include <algorithm>
#include <cmath>
#include <extras/IconsFontAwesome6.h>
#include <imgui.h>
#include <iostream>
#include <misc/cpp/imgui_stdlib.h>
#include <rlImGui.h>

namespace Editor {

bool AnimationPluginEditor::OnImGuiRender(Engine::Entity entity, Engine::Registry& registry) {
    if (!registry.HasComponent<Engine::Animation>(entity)) {
        return false;
    }

    auto& animation = registry.GetComponent<Engine::Animation>(entity);
    auto isOpen = ImGui::CollapsingHeader((std::string(GetIcon()) + " " + GetName()).c_str(),
                                          ImGuiTreeNodeFlags_DefaultOpen);

    if (isOpen) {
        ImGui::TextDisabled("Registered Clips:");
        for (const auto& [name, clip] : animation.clips) {
            ImGui::Text(" - %s", name.c_str());
        }
        ImGui::Separator();

        static std::string newAnimBuffer;
        ImGui::SetNextItemWidth(-40.0f);
        bool isEnterPressed =
            ImGui::InputText("##NewAnim", &newAnimBuffer, ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::SameLine();
        bool isButtonPressed = ImGui::Button("+", ImVec2(32.0f, 0));

        if ((isEnterPressed || isButtonPressed) && !newAnimBuffer.empty()) {
            if (animation.clips.find(newAnimBuffer) == animation.clips.end()) {
                animation.clips[newAnimBuffer] = Engine::AnimationClip(newAnimBuffer, {});
                if (animation.currentAnimation.empty()) {
                    animation.Play(newAnimBuffer);
                }
            }
            newAnimBuffer.clear();
        }

        ImGui::Separator();

        static bool showAnimatorWindow = false;
        if (ImGui::Button("Open Animator Editor", ImVec2(-1.0f, 0))) {
            showAnimatorWindow = true;
        }

        if (showAnimatorWindow) {
            ImGui::SetNextWindowSize(ImVec2(850, 750), ImGuiCond_Once);

            if (ImGui::Begin("Animation Editor", &showAnimatorWindow)) {
                Engine::AnimationClip* activeClip = animation.GetCurrentClip();

                ImGui::TextDisabled("ANIMATION SELECTION");
                std::string currentAnimName =
                    animation.currentAnimation.empty() ? "None" : animation.currentAnimation;

                ImGui::SetNextItemWidth(std::max(1.0f, ImGui::GetContentRegionAvail().x - 120.0f));
                if (ImGui::BeginCombo("##ActiveAnimationCombo", currentAnimName.c_str())) {
                    for (const auto& [name, clip] : animation.clips) {
                        const bool isSelected = (animation.currentAnimation == name);
                        if (ImGui::Selectable(name.c_str(), isSelected)) {
                            animation.Play(name);
                        }
                        if (isSelected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }

                ImGui::SameLine();

                if (ImGui::Button("Delete", ImVec2(110.0f, 0)) &&
                    !animation.currentAnimation.empty()) {
                    animation.clips.erase(animation.currentAnimation);
                    animation.currentAnimation =
                        animation.clips.empty() ? "" : animation.clips.begin()->first;
                    animation.sprites.clear();
                    animation.currentFrame = 0;
                    activeClip = animation.GetCurrentClip();
                }

                ImGui::Separator();

                if (!activeClip) {
                    ImGui::TextDisabled("Create or select a clip to start editing.");
                    ImGui::End();
                    return true;
                }

                ImGui::TextDisabled("PREVIEW");

                ImVec2 availRegion = ImGui::GetContentRegionAvail();
                float previewHeight = std::max(220.0f, availRegion.y - 420.0f);
                ImGui::BeginChild("PreviewWindow", ImVec2(0, previewHeight), true,
                                  ImGuiWindowFlags_NoScrollbar);

                bool isSingleSheet =
                    (activeClip->frameWidth > 0.0f && activeClip->frameHeight > 0.0f) ||
                    !activeClip->frameData.empty();
                std::size_t spriteIdx = isSingleSheet ? 0 : animation.currentFrame;

                if (!animation.sprites.empty() && spriteIdx < animation.sprites.size() &&
                    animation.sprites[spriteIdx]) {
                    auto& sprite = animation.sprites[spriteIdx];
                    const auto& texture = sprite->GetTexture();
                    raylib::Rectangle src = sprite->GetSourceRect();

                    if (!activeClip->frameData.empty() &&
                        animation.currentFrame < activeClip->frameData.size()) {
                        src = activeClip->frameData[animation.currentFrame].rect;
                    } else if (activeClip->frameWidth > 0.0f && activeClip->frameHeight > 0.0f) {
                        int cols =
                            std::max(1, static_cast<int>(texture.width / activeClip->frameWidth));
                        int col = static_cast<int>(animation.currentFrame) % cols;
                        int row = static_cast<int>(animation.currentFrame) / cols;
                        src = raylib::Rectangle{static_cast<float>(col * activeClip->frameWidth),
                                                static_cast<float>(row * activeClip->frameHeight),
                                                activeClip->frameWidth, activeClip->frameHeight};
                    }

                    if (animation.flipX) {
                        src.width = -std::abs(src.width);
                    }
                    if (animation.flipY) {
                        src.height = -std::abs(src.height);
                    }

                    float spriteW =
                        std::abs(src.width > 0 ? src.width : static_cast<float>(texture.width));
                    float spriteH =
                        std::abs(src.height > 0 ? src.height : static_cast<float>(texture.height));

                    ImVec2 viewportSize = ImGui::GetContentRegionAvail();

                    float maxW = std::max(1.0f, viewportSize.x - 20.0f);
                    float maxH = std::max(1.0f, viewportSize.y - 20.0f);

                    float scale = std::min(maxW / spriteW, maxH / spriteH);
                    int targetW = static_cast<int>(spriteW * scale);
                    int targetH = static_cast<int>(spriteH * scale);

                    ImGui::SetCursorPos(
                        ImVec2((viewportSize.x - static_cast<float>(targetW)) * 0.5f,
                               (viewportSize.y - static_cast<float>(targetH)) * 0.5f));

                    ImDrawList* drawList = ImGui::GetWindowDrawList();
                    ImVec2 previewItemMin = ImGui::GetCursorScreenPos();

                    if (src.width != 0 && src.height != 0) {
                        rlImGuiImageRect(&texture, targetW, targetH, src);
                    } else {
                        rlImGuiImageSize(&texture, targetW, targetH);
                    }

                    ImVec2 spriteCenter(previewItemMin.x + static_cast<float>(targetW) * 0.5f,
                                        previewItemMin.y + static_cast<float>(targetH) * 0.5f);
                    ImVec2 pivotPos(spriteCenter.x + (animation.offsetX * scale),
                                    spriteCenter.y + (animation.offsetY * scale));

                    float crossSize = 10.0f;
                    drawList->AddLine(ImVec2(pivotPos.x - crossSize, pivotPos.y),
                                      ImVec2(pivotPos.x + crossSize, pivotPos.y),
                                      IM_COL32(255, 0, 0, 255), 2.0f);
                    drawList->AddLine(ImVec2(pivotPos.x, pivotPos.y - crossSize),
                                      ImVec2(pivotPos.x, pivotPos.y + crossSize),
                                      IM_COL32(0, 255, 0, 255), 2.0f);
                    drawList->AddCircleFilled(pivotPos, 3.0f, IM_COL32(255, 255, 0, 255));
                } else {
                    ImGui::TextDisabled("No texture loaded for current frame.");
                }

                ImGui::EndChild();

                if (ImGui::Button(animation.isPlaying ? "Pause" : "Play")) {
                    animation.isPlaying = !animation.isPlaying;
                }
                ImGui::SameLine();
                ImGui::Checkbox("Loop", &activeClip->loop);

                ImGui::Separator();

                ImGui::TextDisabled("TIMELINE / FRAMES");
                ImGui::BeginChild("SpritesGallery", ImVec2(0, 120.0f), true,
                                  ImGuiWindowFlags_HorizontalScrollbar);

                size_t totalFrameCount = 0;
                if (!activeClip->frameData.empty()) {
                    totalFrameCount = activeClip->frameData.size();
                } else if (activeClip->totalFrames > 0) {
                    totalFrameCount = activeClip->totalFrames;
                } else if (!activeClip->assetPaths.empty()) {
                    totalFrameCount = activeClip->assetPaths.size();
                }

                for (size_t i = 0; i < totalFrameCount; ++i) {
                    ImGui::PushID(static_cast<int>(i));

                    bool isCurrent = (i == animation.currentFrame);
                    if (isCurrent) {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.9f, 0.6f));
                    }

                    std::string frameLabel = "Frame " + std::to_string(i);
                    if (ImGui::Button(frameLabel.c_str(), ImVec2(80.0f, 80.0f))) {
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

                ImGui::TextDisabled("RESOURCES & FILES");
                if (ImGui::Button("Browse Sprites...", ImVec2(160.0f, 0))) {
                    std::vector<std::string> selectedFiles =
                        Editor::PickFilesDialog("Select Sprites");

                    if (!selectedFiles.empty()) {
                        activeClip->assetPaths = std::move(selectedFiles);
                        animation.sprites.clear();
                        animation.currentFrame = 0;
                        animation.timer = 0.0f;
                    }
                }

                if (!activeClip->assetPaths.empty()) {
                    ImGui::SameLine();
                    ImGui::TextWrapped("Source: %s", activeClip->assetPaths[0].c_str());
                }

                ImGui::Separator();
                ImGui::TextDisabled("GRID SLICING (UNIFORM SPRITESHEET)");

                bool widthChanged = ImGui::InputFloat("Frame Width (px)", &activeClip->frameWidth,
                                                      1.0f, 10.0f, "%.0f");
                bool heightChanged = ImGui::InputFloat(
                    "Frame Height (px)", &activeClip->frameHeight, 1.0f, 10.0f, "%.0f");

                if (!animation.sprites.empty() && animation.sprites[0]) {
                    const auto& texture = animation.sprites[0]->GetTexture();

                    if (texture.width > 0 && texture.height > 0 && activeClip->frameWidth > 0.0f &&
                        activeClip->frameHeight > 0.0f) {
                        int cols = static_cast<int>(texture.width / activeClip->frameWidth);
                        int rows = static_cast<int>(texture.height / activeClip->frameHeight);
                        size_t calculatedFrames = static_cast<size_t>(cols * rows);

                        ImGui::Text("Detected: %d columns x %d rows (%zu total frames)", cols, rows,
                                    calculatedFrames);
                        ImGui::SameLine();

                        if (ImGui::Button("Apply Slice") || widthChanged || heightChanged) {
                            activeClip->totalFrames = calculatedFrames;
                            animation.currentFrame = 0;
                        }
                    }
                }

                int totalFrames = static_cast<int>(activeClip->totalFrames);
                if (ImGui::InputInt("Total Frames (0 = manual)", &totalFrames)) {
                    if (totalFrames >= 0) {
                        activeClip->totalFrames = static_cast<size_t>(totalFrames);
                    }
                }

                ImGui::Separator();

                ImGui::TextDisabled("ENTITY TRANSFORM PROPERTIES");
                ImGui::InputFloat("Offset X", &animation.offsetX);
                ImGui::InputFloat("Offset Y", &animation.offsetY);
                ImGui::InputFloat("Scale X", &animation.scaleX);
                ImGui::InputFloat("Scale Y", &animation.scaleY);
                ImGui::Checkbox("Flip X", &animation.flipX);
                ImGui::SameLine();
                ImGui::Checkbox("Flip Y", &animation.flipY);
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