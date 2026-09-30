#ifndef EDITOR_PLUGINEDITOR_HPP_
#define EDITOR_PLUGINEDITOR_HPP_


#include "ECS/Registry.hpp"
#include <nlohmann/json.hpp>

namespace Editor {

/**
 * @class IPluginEditor
 * @brief Interface for defining how an engine component is visualized and serialized in the Editor.
 */
class IPluginEditor {
  public:
    virtual ~IPluginEditor() = default;

    /**
     * @brief The display name of the component in the Inspector.
     */
    virtual const char* GetName() const = 0;

    /**
     * @brief The icon prefix (e.g. FontAwesome or Material Design string)
     */
    virtual const char* GetIcon() const = 0;

    /**
     * @brief Draws the ImGui properties if the entity possesses this component.
     * @return True if the component was drawn, false if the entity lacks the component.
     */
    virtual bool OnImGuiRender(Engine::Entity entity, Engine::Registry& registry) = 0;

    /**
     * @brief Serializes the component data into the parent entity's JSON node.
     */
    virtual void Serialize(Engine::Entity entity, Engine::Registry& registry,
                           nlohmann::json& outEntityJson) = 0;

    /**
     * @brief Deserializes the component data and attaches it to the entity.
     */
    virtual void Deserialize(Engine::Entity entity, Engine::Registry& registry,
                             const nlohmann::json& inEntityJson) = 0;
};

} // namespace Editor
#endif // EDITOR_PLUGINEDITOR_HPP_
