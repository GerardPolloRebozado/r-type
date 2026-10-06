#include "Plugins/SpritePluginEditor.hpp"
#include "Assets/AssetManager.hpp"
#include "Components/SpriteComponent.hpp"
#include <extras/IconsFontAwesome6.h>
#include <imgui.h>
#include <string>

namespace Editor {

const char* SpritePluginEditor::GetName() const {
    return "Sprite";
}

const char* SpritePluginEditor::GetIcon() const {
    return ICON_FA_IMAGE;
}

bool SpritePluginEditor::OnImGuiRender(Engine::Entity entity, Engine::Registry& registry) {
    if (!registry.HasComponent<Engine::SpriteComponent>(entity)) {
        return false;
    }

    auto& spriteComp = registry.GetComponent<Engine::SpriteComponent>(entity);

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
        std::string currentPath = spriteComp.spriteAlias;
        char buffer[256];
        strncpy(buffer, currentPath.c_str(), sizeof(buffer));
        buffer[sizeof(buffer) - 1] = '\0';

        ImGui::Text("Path / Alias");
        ImGui::SameLine();
        if (ImGui::InputText("##SpritePath", buffer, sizeof(buffer),
                             ImGuiInputTextFlags_EnterReturnsTrue)) {
            std::string newPath(buffer);
            if (!newPath.empty() && newPath != currentPath) {
                try {
                    if (registry.HasResource<Engine::AssetManager>()) {
                        registry.GetResource<Engine::AssetManager>().LoadSprite(newPath, newPath);
                        spriteComp.spriteAlias = newPath;
                    }
                } catch (...) {
                    // Could not load sprite
                }
            }
        }

        // Drag and drop target for Sprite path
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload =
                    ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
                const char* path = (const char*)payload->Data;
                std::string newPath(path);
                try {
                    if (registry.HasResource<Engine::AssetManager>()) {
                        registry.GetResource<Engine::AssetManager>().LoadSprite(newPath, newPath);
                        spriteComp.spriteAlias = newPath;
                    }
                } catch (...) {
                    // Could not load sprite
                }
            }
            ImGui::EndDragDropTarget();
        }

        // Offset
        float offset[2] = {spriteComp.offsetX, spriteComp.offsetY};
        if (ImGui::DragFloat2("Offset", offset, 0.1f)) {
            spriteComp.offsetX = offset[0];
            spriteComp.offsetY = offset[1];
        }
    }

    if (remove) {
        RemoveComponent(entity, registry);
        return false;
    }

    return true;
}

void SpritePluginEditor::Serialize(Engine::Entity entity, Engine::Registry& registry,
                                   nlohmann::json& outEntityJson) {
    if (registry.HasComponent<Engine::SpriteComponent>(entity)) {
        auto& spriteComp = registry.GetComponent<Engine::SpriteComponent>(entity);
        std::string path = spriteComp.spriteAlias;
        outEntityJson["SpriteComponent"] = {
            {"path", path}, {"offsetX", spriteComp.offsetX}, {"offsetY", spriteComp.offsetY}};
    }
}

void SpritePluginEditor::Deserialize(Engine::Entity entity, Engine::Registry& registry,
                                     const nlohmann::json& inEntityJson) {
    if (inEntityJson.contains("SpriteComponent")) {
        auto spriteJson = inEntityJson["SpriteComponent"];
        std::string path = spriteJson.value("path", "");
        float offsetX = spriteJson.value("offsetX", 0.0f);
        float offsetY = spriteJson.value("offsetY", 0.0f);

        if (!path.empty()) {
            try {
                if (registry.HasResource<Engine::AssetManager>()) {
                    registry.GetResource<Engine::AssetManager>().LoadSprite(path, path);
                }
            } catch (...) {
            }
        }
        registry.AddComponent<Engine::SpriteComponent>(
            entity, Engine::SpriteComponent(path, offsetX, offsetY));
    }
}

bool SpritePluginEditor::HasComponent(Engine::Entity entity, Engine::Registry& registry) {
    return registry.HasComponent<Engine::SpriteComponent>(entity);
}

void SpritePluginEditor::AddComponent(Engine::Entity entity, Engine::Registry& registry) {
    registry.AddComponent<Engine::SpriteComponent>(entity, Engine::SpriteComponent("", 0.0f, 0.0f));
}

void SpritePluginEditor::RemoveComponent(Engine::Entity entity, Engine::Registry& registry) {
    registry.RemoveComponent<Engine::SpriteComponent>(entity);
}

} // namespace Editor
