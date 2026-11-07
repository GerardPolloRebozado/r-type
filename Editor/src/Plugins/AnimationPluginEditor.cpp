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

    auto& animation = registry.RequireComponent<Engine::Animation>(entity);
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

                    float framePivotX = animation.offsetX;
                    float framePivotY = animation.offsetY;
                    if (!activeClip->frameData.empty() &&
                        animation.currentFrame < activeClip->frameData.size()) {
                        framePivotX += activeClip->frameData[animation.currentFrame].pivotX;
                        framePivotY += activeClip->frameData[animation.currentFrame].pivotY;
                    }

                    ImVec2 pivotPos(spriteCenter.x + (framePivotX * scale),
                                    spriteCenter.y + (framePivotY * scale));

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
                ImGui::SameLine();
                ImGui::SetNextItemWidth(120.0f);
                ImGui::InputFloat("Frame Duration (s)", &activeClip->frameDuration, 0.01f, 0.1f,
                                  "%.3f");

                ImGui::Separator();

                ImGui::TextDisabled("CUSTOM FRAMEDATA CANVAS & EDITOR");

                if (!animation.sprites.empty() && animation.sprites[0]) {
                    const auto& texture = animation.sprites[0]->GetTexture();

                    ImVec2 canvasSize(ImGui::GetContentRegionAvail().x, 220.0f);
                    ImGui::BeginChild("CanvasChild", canvasSize, true,
                                      ImGuiWindowFlags_NoScrollbar);

                    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
                    ImDrawList* drawList = ImGui::GetWindowDrawList();

                    float texW = static_cast<float>(texture.width);
                    float texH = static_cast<float>(texture.height);
                    float scale =
                        std::min((canvasSize.x - 20.0f) / texW, (canvasSize.y - 20.0f) / texH);
                    int displayW = static_cast<int>(texW * scale);
                    int displayH = static_cast<int>(texH * scale);

                    ImVec2 offsetPos(
                        canvasPos.x + (canvasSize.x - static_cast<float>(displayW)) * 0.5f,
                        canvasPos.y + (canvasSize.y - static_cast<float>(displayH)) * 0.5f);

                    ImGui::SetCursorScreenPos(offsetPos);
                    rlImGuiImageSize(&texture, displayW, displayH);

                    ImGui::SetCursorScreenPos(offsetPos);
                    ImGui::InvisibleButton("CanvasOverlay", ImVec2(static_cast<float>(displayW),
                                                                   static_cast<float>(displayH)));

                    bool isHovered = ImGui::IsItemHovered();

                    drawList->AddRect(offsetPos,
                                      ImVec2(offsetPos.x + displayW, offsetPos.y + displayH),
                                      IM_COL32(255, 255, 255, 120));

                    for (size_t idx = 0; idx < activeClip->frameData.size(); ++idx) {
                        const auto& fd = activeClip->frameData[idx];
                        ImVec2 pMin(offsetPos.x + fd.rect.x * scale,
                                    offsetPos.y + fd.rect.y * scale);
                        ImVec2 pMax(pMin.x + fd.rect.width * scale,
                                    pMin.y + fd.rect.height * scale);

                        ImU32 color = (idx == animation.currentFrame) ? IM_COL32(0, 255, 0, 255)
                                                                      : IM_COL32(255, 255, 0, 180);
                        drawList->AddRect(pMin, pMax, color, 0.0f, 0, 2.0f);

                        ImVec2 pPivot(pMin.x + (fd.rect.width * 0.5f + fd.pivotX) * scale,
                                      pMin.y + (fd.rect.height * 0.5f + fd.pivotY) * scale);
                        drawList->AddCircleFilled(pPivot, 2.5f, IM_COL32(255, 0, 0, 255));
                    }

                    static ImVec2 dragStart(0.0f, 0.0f);
                    static bool isDragging = false;

                    if (isHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                        dragStart = ImGui::GetIO().MousePos;
                        isDragging = true;
                    }

                    if (isDragging) {
                        ImVec2 dragCurrent = ImGui::GetIO().MousePos;
                        drawList->AddRect(dragStart, dragCurrent, IM_COL32(255, 0, 0, 255), 0.0f, 0,
                                          1.5f);

                        float curW = std::abs(dragCurrent.x - dragStart.x) / scale;
                        float curH = std::abs(dragCurrent.y - dragStart.y) / scale;
                        char sizeLabel[64];
                        std::snprintf(sizeLabel, sizeof(sizeLabel), "%.0f x %.0f px", curW, curH);

                        ImVec2 textPos(std::max(dragStart.x, dragCurrent.x) + 6.0f,
                                       std::min(dragStart.y, dragCurrent.y));
                        drawList->AddText(textPos, IM_COL32(255, 255, 0, 255), sizeLabel);

                        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                            isDragging = false;

                            float relX1 =
                                (std::min(dragStart.x, dragCurrent.x) - offsetPos.x) / scale;
                            float relY1 =
                                (std::min(dragStart.y, dragCurrent.y) - offsetPos.y) / scale;
                            float relW = std::abs(dragCurrent.x - dragStart.x) / scale;
                            float relH = std::abs(dragCurrent.y - dragStart.y) / scale;

                            if (relW > 4.0f && relH > 4.0f) {
                                activeClip->frameData.emplace_back(relX1, relY1, relW, relH);
                                animation.currentFrame = activeClip->frameData.size() - 1;
                            }
                        }
                    }

                    ImGui::EndChild();

                    if (!activeClip->frameData.empty() &&
                        animation.currentFrame < activeClip->frameData.size()) {
                        auto& currentFd = activeClip->frameData[animation.currentFrame];

                        ImGui::Text("Editing FrameData [%zu]", animation.currentFrame);
                        ImGui::SameLine();
                        if (ImGui::Button("Delete Frame")) {
                            activeClip->frameData.erase(activeClip->frameData.begin() +
                                                        animation.currentFrame);
                            if (animation.currentFrame > 0) {
                                animation.currentFrame--;
                            }
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Clear All Frames")) {
                            activeClip->frameData.clear();
                            animation.currentFrame = 0;
                        }

                        if (!activeClip->frameData.empty()) {
                            ImGui::SetNextItemWidth(100.0f);
                            ImGui::InputFloat("X##fdX", &currentFd.rect.x, 1.0f, 10.0f, "%.0f");
                            ImGui::SameLine();
                            ImGui::SetNextItemWidth(100.0f);
                            ImGui::InputFloat("Y##fdY", &currentFd.rect.y, 1.0f, 10.0f, "%.0f");
                            ImGui::SameLine();
                            ImGui::SetNextItemWidth(100.0f);
                            ImGui::InputFloat("W##fdW", &currentFd.rect.width, 1.0f, 10.0f, "%.0f");
                            ImGui::SameLine();
                            ImGui::SetNextItemWidth(100.0f);
                            ImGui::InputFloat("H##fdH", &currentFd.rect.height, 1.0f, 10.0f,
                                              "%.0f");

                            ImGui::SetNextItemWidth(120.0f);
                            ImGui::InputFloat("Pivot X##fdPX", &currentFd.pivotX, 1.0f, 5.0f,
                                              "%.1f");
                            ImGui::SameLine();
                            ImGui::SetNextItemWidth(120.0f);
                            ImGui::InputFloat("Pivot Y##fdPY", &currentFd.pivotY, 1.0f, 5.0f,
                                              "%.1f");
                        }
                    }
                }

                ImGui::Separator();

                ImGui::TextDisabled("INDIVIDUAL FRAME LIST MANAGEMENT");

                if (ImGui::Button("Add Single Frame...")) {
                    auto files = Editor::PickFilesDialog("Select Single Frame");
                    if (!files.empty()) {
                        activeClip->assetPaths.push_back(files[0]);
                        animation.sprites.clear();
                    }
                }

                ImGui::BeginChild("FrameListControl", ImVec2(0, 140.0f), true);
                for (int i = 0; i < static_cast<int>(activeClip->assetPaths.size()); ++i) {
                    ImGui::PushID(i);

                    bool isSelected = (static_cast<size_t>(i) == animation.currentFrame);
                    std::string label =
                        "Frame " + std::to_string(i) + ": " + activeClip->assetPaths[i];

                    if (ImGui::Selectable(
                            label.c_str(), isSelected, 0,
                            ImVec2(std::max(1.0f, ImGui::GetContentRegionAvail().x - 40.0f), 0))) {
                        animation.currentFrame = static_cast<size_t>(i);
                    }

                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                        ImGui::SetDragDropPayload("FRAME_REORDER", &i, sizeof(int));
                        ImGui::Text("Moving Frame %d", i);
                        ImGui::EndDragDropSource();
                    }

                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* payload =
                                ImGui::AcceptDragDropPayload("FRAME_REORDER")) {
                            int sourceIdx = *static_cast<const int*>(payload->Data);
                            std::swap(activeClip->assetPaths[sourceIdx], activeClip->assetPaths[i]);
                            animation.sprites.clear();
                        }
                        ImGui::EndDragDropTarget();
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("X", ImVec2(30.0f, 0))) {
                        activeClip->assetPaths.erase(activeClip->assetPaths.begin() + i);
                        animation.sprites.clear();
                        if (animation.currentFrame >= activeClip->assetPaths.size() &&
                            animation.currentFrame > 0) {
                            animation.currentFrame--;
                        }
                        ImGui::PopID();
                        break;
                    }

                    ImGui::PopID();
                }
                ImGui::EndChild();

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