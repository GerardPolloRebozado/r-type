#ifndef EDITOR_PLUGINS_TRANSFORMPLUGINEDITOR_HPP_
#define EDITOR_PLUGINS_TRANSFORMPLUGINEDITOR_HPP_

#include "Components/Transform.hpp"
#include "PluginEditor.hpp"
#include <extras/IconsFontAwesome6.h>
#include <imgui.h>
#include <string>

namespace Editor {

class TransformPluginEditor : public IPluginEditor {
  public:
    const char* GetName() const override {
        return "Transform";
    }

    const char* GetIcon() const override {
        return ICON_FA_ARROWS_UP_DOWN_LEFT_RIGHT;
    }

    bool OnImGuiRender(Engine::Entity entity, Engine::Registry& registry) override {
        // Only draw if the entity actually has a Transform component
        if (!registry.HasComponent<Engine::Transform>(entity)) {
            return false;
        }

        auto& transform = registry.GetComponent<Engine::Transform>(entity);

        std::string header = std::string(GetIcon()) + " " + GetName();
        if (ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
            // Transform only has X and Y since it's a 2D engine
            float pos[2] = {transform.x, transform.y};
            if (ImGui::DragFloat2("Position", pos, 0.1f)) {
                transform.x = pos[0];
                transform.y = pos[1];
            }
        }
        return true;
    }

    void Serialize(Engine::Entity entity, Engine::Registry& registry,
                   nlohmann::json& outEntityJson) override {
        if (registry.HasComponent<Engine::Transform>(entity)) {
            auto& transform = registry.GetComponent<Engine::Transform>(entity);
            outEntityJson["TransformComponent"] = {{"x", transform.x}, {"y", transform.y}};
        }
    }

    void Deserialize(Engine::Entity entity, Engine::Registry& registry,
                     const nlohmann::json& inEntityJson) override {
        if (inEntityJson.contains("TransformComponent")) {
            auto transformJson = inEntityJson["TransformComponent"];
            float x = transformJson.value("x", 0.0f);
            float y = transformJson.value("y", 0.0f);
            registry.AddComponent<Engine::Transform>(entity, Engine::Transform(x, y));
        }
    }
};

} // namespace Editor
#endif // EDITOR_PLUGINS_TRANSFORMPLUGINEDITOR_HPP_
