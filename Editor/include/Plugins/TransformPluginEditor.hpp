#ifndef EDITOR_PLUGINS_TRANSFORMPLUGINEDITOR_HPP_
#define EDITOR_PLUGINS_TRANSFORMPLUGINEDITOR_HPP_

#include "Components/Position.hpp"
#include "PluginEditor.hpp"
#include <extras/IconsFontAwesome6.h>
#include <imgui.h>
#include <string>

namespace Editor {

class TransformPluginEditor : public IPluginEditor {
  public:
    const char* GetName() const override {
        return "Position";
    }

    const char* GetIcon() const override {
        return ICON_FA_ARROWS_UP_DOWN_LEFT_RIGHT;
    }

    bool OnImGuiRender(Engine::Entity entity, Engine::Registry& registry) override {
        // Only draw if the entity actually has a Position component
        if (!registry.HasComponent<Engine::Position>(entity)) {
            return false;
        }

        auto& position = registry.GetComponent<Engine::Position>(entity);

        std::string header = std::string(GetIcon()) + " " + GetName();
        if (ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
            float pos[2] = {position.x, position.y};
            if (ImGui::DragFloat2("Position", pos, 0.1f)) {
                position.x = pos[0];
                position.y = pos[1];
            }
        }
        return true;
    }

    void Serialize(Engine::Entity entity, Engine::Registry& registry,
                   nlohmann::json& outEntityJson) override {
        if (registry.HasComponent<Engine::Position>(entity)) {
            auto& position = registry.GetComponent<Engine::Position>(entity);
            outEntityJson["PositionComponent"] = {{"x", position.x}, {"y", position.y}};
        }
    }

    void Deserialize(Engine::Entity entity, Engine::Registry& registry,
                     const nlohmann::json& inEntityJson) override {
        if (inEntityJson.contains("PositionComponent")) {
            auto positionJson = inEntityJson["PositionComponent"];
            float x = positionJson.value("x", 0.0f);
            float y = positionJson.value("y", 0.0f);
            registry.AddComponent<Engine::Position>(entity, Engine::Position(x, y));
        } else if (inEntityJson.contains("TransformComponent")) {
            auto positionJson = inEntityJson["TransformComponent"];
            float x = positionJson.value("x", 0.0f);
            float y = positionJson.value("y", 0.0f);
            registry.AddComponent<Engine::Position>(entity, Engine::Position(x, y));
        }
    }
};

} // namespace Editor
#endif // EDITOR_PLUGINS_TRANSFORMPLUGINEDITOR_HPP_
