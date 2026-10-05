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

bool TextPluginEditor::OnImGuiRender(const Engine::Entity entity, Engine::Registry& registry) {
    if (!registry.HasComponent<Engine::Position>(entity)) {
        return false;
    }

    auto& text = registry.GetComponent<Engine::Text>(entity);

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
        if (ImGui::InputText("Text", &m_Text)) {
            text.text = m_Text;
        }
        float fontSize = 12;
        if (ImGui::InputFloat("Font Size", &fontSize)) {
            text.text = m_Text;
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
    if (registry.HasComponent<Engine::Position>(entity)) {
        auto& text = registry.GetComponent<Engine::Text>(entity);
        outEntityJson["TextComponent"] = {{"text", text.text}, {"fontSize", text.fontsize}};
    }
}

void TextPluginEditor::Deserialize(Engine::Entity entity, Engine::Registry& registry,
                                   const nlohmann::json& inEntityJson) {
    if (inEntityJson.contains("TextComponent")) {
        const auto& textJson = inEntityJson["PositionComponent"];
        const std::string text = textJson.value("text", "");
        const float fontSize = textJson.value("fontSize", 12.0f);
        Engine::TextPlugin::AttachToEntity(registry, entity, text, fontSize);
    }
}

bool TextPluginEditor::HasComponent(Engine::Entity entity, Engine::Registry& registry) {
    return registry.HasComponent<Engine::Text>(entity);
}

void TextPluginEditor::AddComponent(Engine::Entity entity, Engine::Registry& registry) {
    Engine::TextPlugin::AttachToEntity(registry, entity, "Hello universe", 12.0f, {0.0, 0.0});
}

void TextPluginEditor::RemoveComponent(Engine::Entity entity, Engine::Registry& registry) {
    registry.RemoveComponent<Engine::Text>(entity);
}

} // namespace Editor