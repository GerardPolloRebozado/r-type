#include "Plugins/UiPluginEditor.hpp"
#include "Assets/ProjectPaths.hpp"
#include "UI/UiPlugin.hpp"
#include <extras/IconsFontAwesome6.h>
#include <filesystem>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <optional>
#include <string>

namespace Editor {

namespace {

// ImGui owns the payload for this call. The path is copied out and the pointer is not kept.
std::optional<std::string> AcceptBrowserPath() {
    const ImGuiPayload* dropped = nullptr;

    dropped = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM");
    if (dropped == nullptr || dropped->Data == nullptr) {
        return std::nullopt;
    }
    return std::string(static_cast<const char*>(dropped->Data));
}

} // namespace

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
    // The loaded table belongs to the previous file. Drop it so the next frame reloads.
    if (ImGui::InputText("Script", &ui.scriptPath)) {
        ui.loaded = false;
        ui.scriptTable = sol::lua_nil;
    }

    // The drop payload is an absolute path. Scenes store a project-relative one.
    if (ImGui::BeginDragDropTarget()) {
        std::optional<std::string> dropped = AcceptBrowserPath();

        if (dropped.has_value()) {
            ui.scriptPath = ToProjectRelativePath(registry, *dropped);
            ui.loaded = false;
            ui.scriptTable = sol::lua_nil;
        }
        ImGui::EndDragDropTarget();
    }

    ImGui::Checkbox("Auto Reload", &ui.autoReload);

    // The runtime logs a missing file once. The inspector shows it on every frame.
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
