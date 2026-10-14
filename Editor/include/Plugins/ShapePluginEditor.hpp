#ifndef EDITOR_PLUGINS_SHAPEPLUGINEDITOR_HPP_
#define EDITOR_PLUGINS_SHAPEPLUGINEDITOR_HPP_

#include "Components/ShapeComponent.hpp"
#include "PluginEditor.hpp"

namespace Editor {

/**
 * @class ShapePluginEditor
 * @brief Inspector editor for the ShapeComponent.
 */
class ShapePluginEditor : public IPluginEditor {
  public:
    const char* GetName() const override;
    const char* GetIcon() const override;
    bool OnImGuiRender(Engine::Entity entity, Engine::Registry& registry) override;
    bool HasComponent(Engine::Entity entity, Engine::Registry& registry) override;
    void AddComponent(Engine::Entity entity, Engine::Registry& registry) override;
    void RemoveComponent(Engine::Entity entity, Engine::Registry& registry) override;

    /**
     * @brief Draws the editable shape controls.
     * @param shape Shape to edit.
     * @param showVisualProps Whether to expose render-only properties.
     * @return True when a field changed.
     */
    static bool DrawShapeControls(Engine::ShapeComponent& shape, bool showVisualProps = true);
};

} // namespace Editor

#endif // EDITOR_PLUGINS_SHAPEPLUGINEDITOR_HPP_
