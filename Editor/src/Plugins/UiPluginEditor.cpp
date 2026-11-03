#include "Plugins/UiPluginEditor.hpp"
#include "Assets/ProjectPaths.hpp"
#include "UI/UiPlugin.hpp"
#include <extras/IconsFontAwesome6.h>
#include <filesystem>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace Editor {

const char* UiPluginEditor::GetName() const {
    return "UI";
}

const char* UiPluginEditor::GetIcon() const {
    return ICON_FA_DISPLAY;
}

bool UiPluginEditor::OnImGuiRender(Engine::Entity entity, Engine::Registry& registry) {
    std::string header;
    bool isOpen = false;

    if (!registry.IsComponentRegistered<Engine::UiComponent>() ||
        !registry.HasComponent<Engine::UiComponent>(entity)) {
        return false;
    }

    auto& ui = registry.RequireComponent<Engine::UiComponent>(entity);
    header = std::string(GetIcon()) + " " + GetName();
    isOpen = ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen);

    if (ImGui::BeginPopupContextItem()) {
        if (ImGui::MenuItem("Remove Component")) {
            RemoveComponent(entity, registry);
            ImGui::EndPopup();
            return false;
        }
        ImGui::EndPopup();
    }

    if (!isOpen) {
        return true;
    }

    ImGui::SetNextItemWidth(220.0f);
    if (ImGui::InputText("Script", &ui.scriptPath)) {
        ui.loaded = false;
        ui.scriptTable = sol::lua_nil;
    }

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
            const char* path = static_cast<const char*>(payload->Data);
            ui.scriptPath = ToProjectRelativePath(registry, path);
            ui.loaded = false;
            ui.scriptTable = sol::lua_nil;
        }
        ImGui::EndDragDropTarget();
    }

    ImGui::Checkbox("Auto Reload", &ui.autoReload);

    if (!ui.scriptPath.empty() &&
        !std::filesystem::exists(ResolveAssetPath(registry, ui.scriptPath))) {
        ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.4f, 1.0f), "File not found");
    }

    return true;
}

bool UiPluginEditor::HasComponent(Engine::Entity entity, Engine::Registry& registry) {
    if (!registry.IsComponentRegistered<Engine::UiComponent>()) {
        return false;
    }
    return registry.HasComponent<Engine::UiComponent>(entity);
}

void UiPluginEditor::AddComponent(Engine::Entity entity, Engine::Registry& registry) {
    Engine::UiPlugin::AttachToEntity(registry, entity, Engine::UiComponent{});
}

void UiPluginEditor::RemoveComponent(Engine::Entity entity, Engine::Registry& registry) {
    if (!registry.IsComponentRegistered<Engine::UiComponent>()) {
        return;
    }
    registry.RemoveComponent<Engine::UiComponent>(entity);
}

} // namespace Editor
