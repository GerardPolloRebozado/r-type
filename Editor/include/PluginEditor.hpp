#ifndef EDITOR_PLUGINEDITOR_HPP_
#define EDITOR_PLUGINEDITOR_HPP_

#include "ECS/Registry.hpp"

namespace Editor {

/**
 * @class IPluginEditor
 * @brief Interface for defining how an engine component is visualized in the Editor Inspector.
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
     * @brief Checks if the entity has this component.
     */
    virtual bool HasComponent(Engine::Entity entity, Engine::Registry& registry) = 0;

    /**
     * @brief Adds a default-initialized instance of the component to the entity.
     */
    virtual void AddComponent(Engine::Entity entity, Engine::Registry& registry) = 0;

    /**
     * @brief Removes the component from the entity.
     */
    virtual void RemoveComponent(Engine::Entity entity, Engine::Registry& registry) = 0;
};

} // namespace Editor
#endif // EDITOR_PLUGINEDITOR_HPP_
