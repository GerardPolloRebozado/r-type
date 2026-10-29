#include "Plugins/PositionPluginEditor.hpp"
#include "Components/Parent.hpp"
#include "Components/Position.hpp"
#include <extras/IconsFontAwesome6.h>
#include <imgui.h>
#include <string>

namespace Editor {
namespace {
// True when the entity follows a live parent that has a Position, so its
// inspector values should be shown relative to that parent.
bool HasRelativeParent(Engine::Entity entity, Engine::Registry& registry) {
    if (!registry.IsComponentRegistered<Engine::Parent>() ||
        !registry.HasComponent<Engine::Parent>(entity)) {
        return false;
    }
    if (!registry.RequireComponent<Engine::Position>(entity).relative) {
        return false;
    }
    Engine::Entity const parent = registry.RequireComponent<Engine::Parent>(entity).parent;
    return registry.IsEntityAlive(parent) && registry.HasComponent<Engine::Position>(parent);
}
} // namespace

const char* PositionPluginEditor::GetName() const {
    return "Position";
}

const char* PositionPluginEditor::GetIcon() const {
    return ICON_FA_ARROWS_UP_DOWN_LEFT_RIGHT;
}

bool PositionPluginEditor::OnImGuiRender(Engine::Entity entity, Engine::Registry& registry) {
    if (!registry.HasComponent<Engine::Position>(entity)) {
        return false;
    }

    auto& position = registry.RequireComponent<Engine::Position>(entity);

    std::string header = std::string(GetIcon()) + " " + GetName();
    bool isOpen = ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen);

    if (isOpen) {
        // Position stores world coordinates. For a child that follows its parent,
        // show and edit it as an offset from the parent instead.
        float originX = 0.0f;
        float originY = 0.0f;
        bool const hasParent = HasRelativeParent(entity, registry);
        if (hasParent) {
            Engine::Entity const parent = registry.RequireComponent<Engine::Parent>(entity).parent;
            auto const& parentPosition = registry.RequireComponent<Engine::Position>(parent);
            originX = parentPosition.x;
            originY = parentPosition.y;
        }

        float pos[2] = {position.x - originX, position.y - originY};
        if (ImGui::DragFloat2(hasParent ? "Local Position" : "Position", pos, 0.1f)) {
            position.x = pos[0] + originX;
            position.y = pos[1] + originY;
        }
        if (hasParent) {
            ImGui::TextDisabled("World: (%.1f, %.1f)", position.x, position.y);
        }
        ImGui::DragInt("Z Index", &position.z);
    }

    return true;
}

bool PositionPluginEditor::HasComponent(Engine::Entity entity, Engine::Registry& registry) {
    return registry.HasComponent<Engine::Position>(entity);
}

void PositionPluginEditor::AddComponent(Engine::Entity entity, Engine::Registry& registry) {
    registry.AddComponent<Engine::Position>(entity, Engine::Position(0, 0, 0));
}

void PositionPluginEditor::RemoveComponent(Engine::Entity entity, Engine::Registry& registry) {
    registry.RemoveComponent<Engine::Position>(entity);
}

} // namespace Editor
