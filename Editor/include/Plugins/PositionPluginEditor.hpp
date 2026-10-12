#ifndef EDITOR_PLUGINS_POSITIONPLUGINEDITOR_HPP_
#define EDITOR_PLUGINS_POSITIONPLUGINEDITOR_HPP_

#include "PluginEditor.hpp"

namespace Editor {

/**
 * @class PositionPluginEditor
 * @brief Editor plugin for managing and visualizing the Position component.
 *
 * This class provides the UI logic to display, edit, add, and remove
 * the Position component within the Editor's Inspector Panel.
 * It also handles JSON serialization and deserialization for the component.
 */
class PositionPluginEditor : public IPluginEditor {
  public:
    /**
     * @brief Gets the display name of the plugin.
     * @return The string "Position".
     */
    const char* GetName() const override;

    /**
     * @brief Gets the FontAwesome icon associated with the plugin.
     * @return The icon string.
     */
    const char* GetIcon() const override;

    /**
     * @brief Renders the ImGui interface for the Position component.
     * @param entity The entity currently selected in the editor.
     * @param registry The ECS registry containing the entity.
     * @return True if the component was rendered, false if the entity lacks the component.
     */
    bool OnImGuiRender(Engine::Entity entity, Engine::Registry& registry) override;

    /**
     * @brief Checks if the given entity possesses a Position component.
     * @param entity The entity to check.
     * @param registry The ECS registry.
     * @return True if the entity has a Position component, false otherwise.
     */
    bool HasComponent(Engine::Entity entity, Engine::Registry& registry) override;

    /**
     * @brief Adds a default-initialized Position component to the entity.
     * @param entity The entity to add the component to.
     * @param registry The ECS registry.
     */
    void AddComponent(Engine::Entity entity, Engine::Registry& registry) override;

    /**
     * @brief Removes the Position component from the entity.
     * @param entity The entity to remove the component from.
     * @param registry The ECS registry.
     */
    void RemoveComponent(Engine::Entity entity, Engine::Registry& registry) override;
};

} // namespace Editor
#endif // EDITOR_PLUGINS_POSITIONPLUGINEDITOR_HPP_