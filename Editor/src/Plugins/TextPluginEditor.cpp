#include "Plugins/TextPluginEditor.hpp"

#include "Assets/ProjectPaths.hpp"
#include "Components/Text.hpp"
#include "Logging/Logger.hpp"
#include "Plugins/TextPlugin.hpp"

#include "misc/cpp/imgui_stdlib.h"
#include <cstring>
#include <extras/IconsFontAwesome6.h>
#include <filesystem>
#include <imgui.h>
#include <string>

namespace Editor {

namespace {

void LoadFontForComponent(Engine::Registry& registry, Engine::Text& textComp,
                          const std::string& inputPath) {
    if (inputPath.empty()) {
        if (textComp.font.texture.id != 0) {
            UnloadFont(textComp.font);
            textComp.font = {};
        }
        textComp.fontPath = "";
        return;
    }

    std::string relPath = ToProjectRelativePath(registry, inputPath);
    auto resolvedPath = ResolveAssetPath(registry, relPath);

    if (!std::filesystem::exists(resolvedPath)) {
        log_user_error("[TextPluginEditor] Font file not found: " + resolvedPath.string());
        return;
    }

    Font newFont = LoadFont(resolvedPath.string().c_str());
    if (newFont.texture.id != 0) {
        if (textComp.font.texture.id != 0) {
            UnloadFont(textComp.font);
        }
        textComp.font = newFont;
        textComp.fontPath = relPath;
    } else {
        log_user_error("[TextPluginEditor] Failed to load font: " + resolvedPath.string());
    }
}

} // namespace

const char* TextPluginEditor::GetName() const {
    return "Text";
}

const char* TextPluginEditor::GetIcon() const {
    return ICON_FA_FONT;
}

bool TextPluginEditor::OnImGuiRender(Engine::Entity entity, Engine::Registry& registry) {
    if (!registry.HasComponent<Engine::Text>(entity)) {
        return false;
    }

    auto& text = registry.RequireComponent<Engine::Text>(entity);

    const std::string header = std::string(GetIcon()) + " " + GetName();
    const bool isOpen = ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen);

    bool remove = false;
    if (ImGui::BeginPopupContextItem()) {
        if (ImGui::MenuItem("Remove Component")) {
            remove = true;
        }
        ImGui::EndPopup();
    }

    if (isOpen) {
        ImGui::InputText("Text", &text.text);

        // Font Path & Drag/Drop
        std::string currentPath = text.fontPath;
        char buffer[256];
        std::strncpy(buffer, currentPath.c_str(), sizeof(buffer));
        buffer[sizeof(buffer) - 1] = '\0';

        ImGui::Text("Font");
        ImGui::SameLine();
        if (ImGui::InputText("##FontPath", buffer, sizeof(buffer),
                             ImGuiInputTextFlags_EnterReturnsTrue) ||
            ImGui::IsItemDeactivatedAfterEdit()) {
            std::string newPath(buffer);
            if (newPath != currentPath) {
                LoadFontForComponent(registry, text, newPath);
            }
        }

        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload =
                    ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
                const char* droppedPath = static_cast<const char*>(payload->Data);
                LoadFontForComponent(registry, text, droppedPath);
            }
            ImGui::EndDragDropTarget();
        }

        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_TRASH "##ClearFont")) {
            LoadFontForComponent(registry, text, "");
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Reset to default engine font");
        }

        if (!text.fontPath.empty()) {
            auto resolved = ResolveAssetPath(registry, text.fontPath);
            if (!std::filesystem::exists(resolved)) {
                ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.4f, 1.0f), "Font file not found on disk");
            } else {
                ImGui::TextDisabled("Custom font active");
            }
        }

        // Font Size
        int fontSize = static_cast<int>(text.fontsize);
        if (ImGui::InputInt("Font Size", &fontSize)) {
            if (fontSize > 0) {
                text.fontsize = static_cast<float>(fontSize);
            }
        }

        // Spacing
        ImGui::DragFloat("Spacing", &text.spacing, 0.1f, 0.0f, 50.0f, "%.1f");

        // Color
        float colorF[4] = {text.color.r / 255.0f, text.color.g / 255.0f, text.color.b / 255.0f,
                           text.color.a / 255.0f};
        if (ImGui::ColorEdit4("Color", colorF)) {
            text.color.r = static_cast<unsigned char>(colorF[0] * 255.0f);
            text.color.g = static_cast<unsigned char>(colorF[1] * 255.0f);
            text.color.b = static_cast<unsigned char>(colorF[2] * 255.0f);
            text.color.a = static_cast<unsigned char>(colorF[3] * 255.0f);
        }
    }

    if (remove) {
        RemoveComponent(entity, registry);
        return false;
    }

    return true;
}

bool TextPluginEditor::HasComponent(Engine::Entity entity, Engine::Registry& registry) {
    return registry.HasComponent<Engine::Text>(entity);
}

void TextPluginEditor::AddComponent(Engine::Entity entity, Engine::Registry& registry) {
    Engine::TextPlugin::AttachToEntity(registry, entity, "Hello universe", 12.0f);
}

void TextPluginEditor::RemoveComponent(Engine::Entity entity, Engine::Registry& registry) {
    if (registry.HasComponent<Engine::Text>(entity)) {
        auto& text = registry.RequireComponent<Engine::Text>(entity);
        if (text.font.texture.id != 0) {
            UnloadFont(text.font);
            text.font = {};
        }
    }
    registry.RemoveComponent<Engine::Text>(entity);
}

} // namespace Editor
