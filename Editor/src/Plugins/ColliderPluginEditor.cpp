/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** ColliderPluginEditor.cpp
*/

#include "Plugins/ColliderPluginEditor.hpp"
#include "Components/ColliderComponent.hpp"
#include "Plugins/ColliderPlugin.hpp"
#include "Plugins/ShapePluginEditor.hpp"

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
        ImGui::SameLine();
        bool isEnabled = collider.enabled;
        if (ImGui::Checkbox("Enabled", &isEnabled)) {
            collider.enabled = isEnabled;
        }

        ImGui::Separator();
        ImGui::TextUnformatted("Collision Boundary");

        bool hasCustomShape = collider.shape.has_value();
        if (ImGui::Checkbox("Custom Shape Override", &hasCustomShape)) {
            if (hasCustomShape) {
                if (registry.HasComponent<Engine::ShapeComponent>(entity)) {
                    collider.shape = registry.GetComponent<Engine::ShapeComponent>(entity);
                } else {
                    collider.shape = Engine::ShapeComponent{};
                }
            } else {
                collider.shape = std::nullopt;
            }
        }

        if (collider.shape.has_value()) {
            ImGui::Indent();
            ShapePluginEditor::DrawShapeControls(*collider.shape, false);
            if (ImGui::Button("Reset to Entity Shape")) {
                collider.shape = std::nullopt;
            }
            ImGui::Unindent();
        } else {
            if (registry.HasComponent<Engine::ShapeComponent>(entity)) {
                const auto& entShape = registry.GetComponent<Engine::ShapeComponent>(entity);
                const char* typeNames[] = {"Rectangle", "Circle", "Polygon", "Line"};
                int t = static_cast<int>(entShape.type);
                const char* tName = (t >= 0 && t < 4) ? typeNames[t] : "Unknown";
                ImGui::TextDisabled("Using Entity's Shape (%s)", tName);
            } else {
                ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.0f, 1.0f),
                                   "Warning: No ShapeComponent on Entity!");
                if (ImGui::Button("Create Custom Shape")) {
                    collider.shape = Engine::ShapeComponent{};
                }
            }
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
        nlohmann::json colliderJson = {
            {"isTrigger", collider.isTrigger},
            {"enabled", collider.enabled},
        };
        if (collider.shape.has_value()) {
            colliderJson["shape"] = ShapePluginEditor::SerializeShape(*collider.shape);
        }
        outEntityJson["ColliderComponent"] = colliderJson;
    }
}

void ColliderPluginEditor::Deserialize(const Engine::Entity entity, Engine::Registry& registry,
                                       const nlohmann::json& inEntityJson) {
    if (inEntityJson.contains("ColliderComponent")) {
        const auto& json = inEntityJson["ColliderComponent"];
        Engine::ColliderComponent collider;
        collider.isTrigger = json.value("isTrigger", false);
        collider.enabled = json.value("enabled", true);
        if (json.contains("shape")) {
            collider.shape = ShapePluginEditor::DeserializeShape(json["shape"]);
        }
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
    registry.RemoveComponent<Engine::ColliderComponent>(entity);
}
} // namespace Editor