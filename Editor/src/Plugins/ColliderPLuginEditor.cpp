/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** ColliderPLuginEditor.cpp
*/

#include "Plugins/ColliderPluginEditor.hpp"
#include "Components/ColliderComponent.hpp"
#include "Plugins/ColliderPlugin.hpp"

#include <extras/IconsFontAwesome6.h>
#include <imgui.h>

namespace Editor {

const char* ColliderPluginEditor::GetName() const {
    return "Collider";
}

const char* ColliderPluginEditor::GetIcon() const {
    return ICON_FA_COMPRESS;
}

bool ColliderPluginEditor::OnImGuiRender(const Engine::Entity entity, Engine::Registry& registry) {
    if (!registry.HasComponent<Engine::ColliderComponent>(entity)) {
        return false;
    }
    auto& collider = registry.GetComponent<Engine::ColliderComponent>(entity);

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
        bool isTrigger = collider.isTrigger;
        if (ImGui::Checkbox("Is Trigger", &isTrigger)) {
            collider.isTrigger = isTrigger;
        }
        bool isEnabled = collider.enabled;
        if (ImGui::Checkbox("Enabled", &isEnabled)) {
            collider.enabled = isEnabled;
        }
    }

    if (remove) {
        RemoveComponent(entity, registry);
        return false;
    }

    return true;
}

void ColliderPluginEditor::Serialize(const Engine::Entity entity, Engine::Registry& registry,
                                     nlohmann::json& outEntityJson) {
    if (registry.HasComponent<Engine::ColliderComponent>(entity)) {
        auto& collider = registry.GetComponent<Engine::ColliderComponent>(entity);
        outEntityJson["ColliderComponent"] = {
            {"isTrigger", collider.isTrigger},
            {"enabled", collider.enabled},
        };
    }
}

void ColliderPluginEditor::Deserialize(const Engine::Entity entity, Engine::Registry& registry,
                                       const nlohmann::json& inEntityJson) {
    if (inEntityJson.contains("ColliderComponent")) {
        const auto& json = inEntityJson["ColliderComponent"];
        Engine::ColliderComponent collider;
        collider.isTrigger = json.value("isTrigger", false);
        collider.enabled = json.value("enabled", true);
        Engine::ColliderPlugin::AttachToEntity(registry, entity, collider);
    }
}

bool ColliderPluginEditor::HasComponent(const Engine::Entity entity, Engine::Registry& registry) {
    return registry.HasComponent<Engine::ColliderComponent>(entity);
}

void ColliderPluginEditor::AddComponent(const Engine::Entity entity, Engine::Registry& registry) {
    const Engine::ColliderComponent defaultCollider;
    Engine::ColliderPlugin::AttachToEntity(registry, entity, defaultCollider);
}

void ColliderPluginEditor::RemoveComponent(const Engine::Entity entity,
                                           Engine::Registry& registry) {
    registry.RemoveComponent<Engine::ColliderPlugin>(entity);
}
} // namespace Editor