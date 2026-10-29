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

#include <cstdint>
#include <string>

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
    auto& collider = registry.RequireComponent<Engine::ColliderComponent>(entity);

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
        bool isStatic = collider.isStatic;
        if (ImGui::Checkbox("Is Static", &isStatic)) {
            collider.isStatic = isStatic;
        }
        ImGui::SameLine();
        bool isEnabled = collider.enabled;
        if (ImGui::Checkbox("Enabled", &isEnabled)) {
            collider.enabled = isEnabled;
        }

        ImGui::Separator();
        ImGui::TextUnformatted("Collision Filter");
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Two colliders only interact when each one's mask\n"
                              "contains the other's layer. Clear the bits you\n"
                              "don't want this entity to touch.");
        }

        const std::string layerPreview = Engine::CollisionLayers::Name(collider.layer);
        if (ImGui::BeginCombo("Layer", layerPreview.c_str())) {
            for (int candidate = 0; candidate < Engine::CollisionLayers::kLayerCount; ++candidate) {
                const bool isSelected = (candidate == collider.layer);
                const std::string label = Engine::CollisionLayers::Name(candidate);
                if (ImGui::Selectable(label.c_str(), isSelected)) {
                    collider.layer = candidate;
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        ImGui::TextUnformatted("Collides with");
        for (int layerIndex = 0; layerIndex < Engine::CollisionLayers::kNamedLayerCount;
             ++layerIndex) {
            const std::uint32_t bit = Engine::CollisionLayers::Bit(layerIndex);
            if (bit == 0u) {
                break;
            }
            bool touches = (collider.mask & bit) != 0u;
            const std::string label = Engine::CollisionLayers::Name(layerIndex);
            ImGui::PushID(layerIndex);
            if (ImGui::Checkbox(label.c_str(), &touches)) {
                collider.mask = touches ? (collider.mask | bit) : (collider.mask & ~bit);
            }
            ImGui::PopID();
            if ((layerIndex % 4) != 3 &&
                layerIndex + 1 < Engine::CollisionLayers::kNamedLayerCount) {
                ImGui::SameLine();
            }
        }

        int maskValue = static_cast<int>(collider.mask);
        if (ImGui::InputInt("Mask (hex, all 32 layers)", &maskValue, 1, 16,
                            ImGuiInputTextFlags_CharsHexadecimal)) {
            collider.mask = static_cast<std::uint32_t>(maskValue);
        }

        if (ImGui::Button("All Layers")) {
            collider.mask = Engine::CollisionLayers::kAllLayers;
        }
        ImGui::SameLine();
        if (ImGui::Button("No Layers")) {
            collider.mask = Engine::CollisionLayers::kNoLayers;
        }

        ImGui::Separator();
        ImGui::TextUnformatted("Collision Boundary");

        bool hasCustomShape = collider.shape.has_value();
        if (ImGui::Checkbox("Custom Shape Override", &hasCustomShape)) {
            if (hasCustomShape) {
                if (registry.HasComponent<Engine::ShapeComponent>(entity)) {
                    collider.shape = registry.RequireComponent<Engine::ShapeComponent>(entity);
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
                const auto& entShape = registry.RequireComponent<Engine::ShapeComponent>(entity);
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
