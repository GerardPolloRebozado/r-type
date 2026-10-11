#ifndef EDITOR_PLUGINS_SHAPEPLUGINEDITOR_HPP_
#define EDITOR_PLUGINS_SHAPEPLUGINEDITOR_HPP_

#include "Components/ShapeComponent.hpp"
#include "PluginEditor.hpp"

namespace Editor {

class ShapePluginEditor : public IPluginEditor {
  public:
    const char* GetName() const override;
    const char* GetIcon() const override;
    bool OnImGuiRender(Engine::Entity entity, Engine::Registry& registry) override;
    void Serialize(Engine::Entity entity, Engine::Registry& registry,
                   nlohmann::json& outEntityJson) override;
    void Deserialize(Engine::Entity entity, Engine::Registry& registry,
                     const nlohmann::json& inEntityJson) override;
    bool HasComponent(Engine::Entity entity, Engine::Registry& registry) override;
    void AddComponent(Engine::Entity entity, Engine::Registry& registry) override;
    void RemoveComponent(Engine::Entity entity, Engine::Registry& registry) override;

    static bool DrawShapeControls(Engine::ShapeComponent& shape, bool showVisualProps = true);
    static nlohmann::json SerializeShape(const Engine::ShapeComponent& shape);
    static Engine::ShapeComponent DeserializeShape(const nlohmann::json& json);
};

} // namespace Editor

#endif // EDITOR_PLUGINS_SHAPEPLUGINEDITOR_HPP_
