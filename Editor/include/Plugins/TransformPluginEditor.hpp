#ifndef EDITOR_PLUGINS_TRANSFORMPLUGINEDITOR_HPP_
#define EDITOR_PLUGINS_TRANSFORMPLUGINEDITOR_HPP_

#include "Components/Position.hpp"
#include "PluginEditor.hpp"
#include <extras/IconsFontAwesome6.h>
#include <imgui.h>
#include <string>

namespace Editor {

/**
 * @class TransformPluginEditor
 * @brief Editor plugin for managing and visualizing the Position component.
 *
 * This class provides the UI logic to display, edit, add, and remove
 * the Position component within the Editor's Inspector Panel.
 * It also handles JSON serialization and deserialization for the component.
 */
class TransformPluginEditor : public IPluginEditor {
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
     * @brief Serializes the Position component into a JSON object.
     * @param entity The entity whose component is being serialized.
     * @param registry The ECS registry containing the entity.
     * @param outEntityJson The JSON object to append the serialized data to.
     */
    void Serialize(Engine::Entity entity, Engine::Registry& registry,
                   nlohmann::json& outEntityJson) override;

    /**
     * @brief Deserializes the Position component from a JSON object.
     * @param entity The entity to attach the deserialized component to.
     * @param registry The ECS registry to add the component to.
     * @param inEntityJson The JSON object containing the component data.
     */
    void Deserialize(Engine::Entity entity, Engine::Registry& registry,
                     const nlohmann::json& inEntityJson) override;

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
#endif // EDITOR_PLUGINS_TRANSFORMPLUGINEDITOR_HPP_
