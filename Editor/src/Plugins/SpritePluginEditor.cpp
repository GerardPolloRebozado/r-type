#include "Plugins/SpritePluginEditor.hpp"
#include "Assets/AssetManager.hpp"
#include "Components/Position.hpp"
#include "Components/Rotation.hpp"
#include "Components/SpriteComponent.hpp"
#include "EditorConfig.hpp"
#include <algorithm>
#include <extras/IconsFontAwesome6.h>
#include <imgui.h>
#include <iostream>
#include <rlImGui.h>
#include <string>

namespace Editor {

const char* SpritePluginEditor::GetName() const {
    return "Sprite";
}

const char* SpritePluginEditor::GetIcon() const {
    return ICON_FA_IMAGE;
}

void LoadSpriteForComponent(Engine::Registry& registry, Engine::SpriteComponent& spriteComp,
                            const std::string& inputPath) {
    if (inputPath.empty()) {
        spriteComp.sprite = nullptr;
        spriteComp.assetPath = "";
        return;
    }
    std::string relPath = ToProjectRelativePath(registry, inputPath);
    auto resolvedPath = ResolveAssetPath(registry, relPath);

    std::shared_ptr<Engine::Sprite> spr = nullptr;
    try {
        if (registry.HasResource<Engine::AssetManager>()) {
            auto& am = registry.GetResource<Engine::AssetManager>();
            try {
                auto existing = am.GetSprite(relPath);
                if (existing && existing->GetTexture().id == 0) {
                    am.UnloadSprite(relPath);
                }
            } catch (...) {
            }
            am.LoadSprite(relPath, resolvedPath.string());
            spr = am.GetSprite(relPath);
        } else {
            spr = std::make_shared<Engine::Sprite>(resolvedPath.string());
        }
    } catch (const std::exception& e) {
        std::cerr << "[SpritePluginEditor] Failed to load sprite: " << resolvedPath.string()
                  << " - " << e.what() << "\n";
    } catch (...) {
        std::cerr << "[SpritePluginEditor] Failed to load sprite: " << resolvedPath.string()
                  << " (Unknown Error)\n";
    }

    if (spr) {
        spriteComp.sprite = spr;
        spriteComp.assetPath = relPath;
        if (spriteComp.scaleX > 0.0f && spriteComp.scaleY > 0.0f) {
            spr->SetScale({spriteComp.scaleX, spriteComp.scaleY});
        }
    }
}

bool SpritePluginEditor::OnImGuiRender(Engine::Entity entity, Engine::Registry& registry) {
    if (!registry.HasComponent<Engine::SpriteComponent>(entity)) {
        return false;
    }

    auto& spriteComp = registry.RequireComponent<Engine::SpriteComponent>(entity);

    std::string header = std::string(GetIcon()) + " " + GetName();
    bool isOpen = ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen);

    bool remove = false;
    if (ImGui::BeginPopupContextItem()) {
        if (ImGui::MenuItem("Remove Component")) {
            remove = true;
        }
        ImGui::EndPopup();
    }

    if (isOpen) {
        // Sprite path
        std::string currentPath = spriteComp.assetPath;
        char buffer[256];
        strncpy(buffer, currentPath.c_str(), sizeof(buffer));
        buffer[sizeof(buffer) - 1] = '\0';

        ImGui::Text("Path");
        ImGui::SameLine();
        if (ImGui::InputText("##SpritePath", buffer, sizeof(buffer),
                             ImGuiInputTextFlags_EnterReturnsTrue) ||
            ImGui::IsItemDeactivatedAfterEdit()) {
            std::string newPath(buffer);
            if (newPath != currentPath) {
                LoadSpriteForComponent(registry, spriteComp, newPath);
            }
        }

        // Drag and drop target for Sprite path
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload =
                    ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
                const char* path = (const char*)payload->Data;
                LoadSpriteForComponent(registry, spriteComp, path);
            }
            ImGui::EndDragDropTarget();
        }

        // Offset
        float offset[2] = {spriteComp.offsetX, spriteComp.offsetY};
        if (ImGui::DragFloat2("Offset", offset, 0.1f)) {
            spriteComp.offsetX = offset[0];
            spriteComp.offsetY = offset[1];
        }

        // Scale
        float scale[2] = {spriteComp.scaleX, spriteComp.scaleY};
        if (ImGui::DragFloat2("Scale", scale, 0.05f, 0.001f, 100.0f)) {
            spriteComp.scaleX = std::max(0.001f, scale[0]);
            spriteComp.scaleY = std::max(0.001f, scale[1]);
            if (spriteComp.sprite) {
                spriteComp.sprite->SetScale({spriteComp.scaleX, spriteComp.scaleY});
            }
        }

        bool flipX = spriteComp.flipX;
        if (ImGui::Checkbox("Flip X", &flipX)) {
            spriteComp.flipX = flipX;
        }
        ImGui::SameLine();
        bool flipY = spriteComp.flipY;
        if (ImGui::Checkbox("Flip Y", &flipY)) {
            spriteComp.flipY = flipY;
        }

        // Preview thumbnail in Inspector
        if (spriteComp.sprite && spriteComp.sprite->GetTexture().id > 0) {
            const auto& tex = spriteComp.sprite->GetTexture();
            float previewWidth = 120.0f;
            float previewHeight = 120.0f;
            if (tex.width > 0 && tex.height > 0) {
                float aspect = static_cast<float>(tex.width) / static_cast<float>(tex.height);
                if (aspect >= 1.0f) {
                    previewHeight = previewWidth / aspect;
                } else {
                    previewWidth = previewHeight * aspect;
                }
            }
            ImGui::Spacing();
            ImGui::Text("Sprite Preview:");
            rlImGuiImageSize(&tex, static_cast<int>(previewWidth), static_cast<int>(previewHeight));
            ImGui::TextDisabled("%d x %d px", tex.width, tex.height);
        }
    }

    if (remove) {
        RemoveComponent(entity, registry);
        return false;
    }

    return true;
}

bool SpritePluginEditor::HasComponent(Engine::Entity entity, Engine::Registry& registry) {
    return registry.HasComponent<Engine::SpriteComponent>(entity);
}

void SpritePluginEditor::AddComponent(Engine::Entity entity, Engine::Registry& registry) {
    std::shared_ptr<Engine::Sprite> nullSprite = nullptr;
    registry.AddComponent<Engine::SpriteComponent>(
        entity, Engine::SpriteComponent(nullSprite, "", 0.0f, 0.0f, 1.0f, 1.0f));
    if (!registry.HasComponent<Engine::Position>(entity)) {
        registry.AddComponent<Engine::Position>(entity, Engine::Position(0.0f, 0.0f, 0.0f));
    }
    if (!registry.HasComponent<Engine::Rotation>(entity)) {
        registry.AddComponent<Engine::Rotation>(entity, Engine::Rotation::Create(0.0f));
    }
}

void SpritePluginEditor::RemoveComponent(Engine::Entity entity, Engine::Registry& registry) {
    registry.RemoveComponent<Engine::SpriteComponent>(entity);
}

} // namespace Editor
