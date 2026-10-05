#include "Plugins/TextPluginEditor.hpp"

#include "Components/Text.hpp"
#include "Plugins/TextPlugin.hpp"

#include "misc/cpp/imgui_stdlib.h"
#include <extras/IconsFontAwesome6.h>
#include <imgui.h>
#include <string>

namespace Editor {

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

    auto& text = registry.GetComponent<Engine::Text>(entity);

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
        int fontSize = static_cast<int>(text.fontsize);
        if (ImGui::InputInt("Font Size", &fontSize)) {
            if (fontSize > 0) {
                text.fontsize = static_cast<float>(fontSize);
            }
        }
    }

    if (remove) {
        RemoveComponent(entity, registry);
        return false;
    }

    return true;
}

void TextPluginEditor::Serialize(Engine::Entity entity, Engine::Registry& registry,
                                 nlohmann::json& outEntityJson) {
    if (registry.HasComponent<Engine::Text>(entity)) {
        auto& text = registry.GetComponent<Engine::Text>(entity);
        outEntityJson["TextComponent"] = {{"text", text.text}, {"fontSize", text.fontsize}};
    }
}

void TextPluginEditor::Deserialize(Engine::Entity entity, Engine::Registry& registry,
                                   const nlohmann::json& inEntityJson) {
    if (inEntityJson.contains("TextComponent")) {
        const auto& textJson = inEntityJson["TextComponent"];
        const std::string text = textJson.value("text", "");
        const float fontSize = textJson.value("fontSize", 12.0f);
        Engine::TextPlugin::AttachToEntity(registry, entity, text, fontSize);
    }
}

bool TextPluginEditor::HasComponent(Engine::Entity entity, Engine::Registry& registry) {
    return registry.HasComponent<Engine::Text>(entity);
}

void TextPluginEditor::AddComponent(Engine::Entity entity, Engine::Registry& registry) {
    Engine::TextPlugin::AttachToEntity(registry, entity, "Hello universe", 12.0f);
}

void TextPluginEditor::RemoveComponent(Engine::Entity entity, Engine::Registry& registry) {
    registry.RemoveComponent<Engine::Text>(entity);
}

} // namespace Editor