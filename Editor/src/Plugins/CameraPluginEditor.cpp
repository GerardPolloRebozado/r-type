#include "Plugins/CameraPluginEditor.hpp"
#include "Components/CameraComponent.hpp"
#include "Plugins/CameraPlugin.hpp"

#include <extras/IconsFontAwesome6.h>
#include <imgui.h>
#include <string>

namespace Editor {

const char* CameraPluginEditor::GetName() const {
    return "Camera";
}

const char* CameraPluginEditor::GetIcon() const {
    return ICON_FA_VIDEO;
}

bool CameraPluginEditor::OnImGuiRender(Engine::Entity entity, Engine::Registry& registry) {
    if (!HasComponent(entity, registry)) {
        return false;
    }

    auto& camera = registry.RequireComponent<Engine::CameraComponent>(entity);

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
        ImGui::Checkbox("Primary", &camera.primary);
        ImGui::DragFloat("Zoom", &camera.zoom, 0.01f, 0.1f, 10.0f);

        ImGui::SeparatorText("Follow");
        int target = camera.followTarget ? static_cast<int>(*camera.followTarget) : -1;
        if (ImGui::InputInt("Target Entity", &target)) {
            if (target < 0) {
                camera.followTarget.reset();
            } else {
                camera.followTarget = static_cast<Engine::Entity>(target);
            }
        }
        ImGui::SetItemTooltip("Entity ID to follow, -1 = no follow");
        ImGui::DragFloat("Follow Speed", &camera.followSpeed, 0.1f, 0.0f, 50.0f);
        ImGui::SetItemTooltip("0 = snap to the target");

        ImGui::SeparatorText("Bounds");
        ImGui::Checkbox("Clamp To Bounds", &camera.clampToBounds);
        if (camera.clampToBounds) {
            ImGui::DragFloat2("Bounds Min", &camera.bounds.x, 1.0f);
            ImGui::DragFloat2("Bounds Size", &camera.bounds.width, 1.0f, 0.0f, 100000.0f);
        }

        ImGui::SeparatorText("Shake");
        ImGui::DragFloat("Magnitude", &camera.shakeMagnitude, 0.5f, 0.0f, 200.0f);
        ImGui::DragFloat("Decay", &camera.shakeDecay, 0.05f, 0.0f, 10.0f);
        if (ImGui::Button("Test Shake")) {
            camera.shakeTrauma = 1.0f;
        }
    }

    if (remove) {
        RemoveComponent(entity, registry);
        return false;
    }

    return true;
}

bool CameraPluginEditor::HasComponent(Engine::Entity entity, Engine::Registry& registry) {
    return registry.IsComponentRegistered<Engine::CameraComponent>() &&
           registry.HasComponent<Engine::CameraComponent>(entity);
}

void CameraPluginEditor::AddComponent(Engine::Entity entity, Engine::Registry& registry) {
    Engine::CameraPlugin::AttachToEntity(registry, entity, Engine::CameraComponent{});
}

void CameraPluginEditor::RemoveComponent(Engine::Entity entity, Engine::Registry& registry) {
    registry.RemoveComponent<Engine::CameraComponent>(entity);
}

} // namespace Editor
