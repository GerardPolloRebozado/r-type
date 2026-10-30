#include "Plugins/OpacityPluginEditor.hpp"
#include "Components/Opacity.hpp"
#include "Plugins/OpacityPlugin.hpp"
#include <algorithm>
#include <extras/IconsFontAwesome6.h>
#include <imgui.h>

namespace Editor {

const char* OpacityPluginEditor::GetName() const {
    return "Opacity";
}

const char* OpacityPluginEditor::GetIcon() const {
    return ICON_FA_CIRCLE_HALF_STROKE;
}

bool OpacityPluginEditor::OnImGuiRender(Engine::Entity entity, Engine::Registry& registry) {
    if (!registry.HasComponent<Engine::Opacity>(entity)) {
        return false;
    }

    auto& opacity = registry.RequireComponent<Engine::Opacity>(entity);

    const std::string header = std::string(GetIcon()) + " " + GetName();
    const bool isOpen = ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen);

    if (isOpen) {
        ImGui::DragFloat("Alpha", &opacity.alpha, 0.01f, 0.0f, 1.0f);
        // Typed input can exceed the drag bounds: keep the invariant.
        opacity.alpha = std::clamp(opacity.alpha, 0.0f, 1.0f);
        ImGui::Checkbox("Inherit to children", &opacity.inheritToChildren);

        // Live preview swatch: white at current alpha over a dark background.
        const ImVec2 p = ImGui::GetCursorScreenPos();
        const ImVec2 size(120.0f, 20.0f);
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddRectFilled(p, ImVec2(p.x + size.x, p.y + size.y), IM_COL32(20, 20, 20, 255));
        drawList->AddRectFilled(p, ImVec2(p.x + size.x, p.y + size.y),
                                IM_COL32(255, 255, 255, static_cast<int>(255.0f * opacity.alpha)));
        ImGui::Dummy(size);
    }

    return true;
}

bool OpacityPluginEditor::HasComponent(Engine::Entity entity, Engine::Registry& registry) {
    return registry.HasComponent<Engine::Opacity>(entity);
}

void OpacityPluginEditor::AddComponent(Engine::Entity entity, Engine::Registry& registry) {
    Engine::OpacityPlugin::AttachToEntity(registry, entity, 1.0f);
}

void OpacityPluginEditor::RemoveComponent(Engine::Entity entity, Engine::Registry& registry) {
    registry.RemoveComponent<Engine::Opacity>(entity);
}

} // namespace Editor
