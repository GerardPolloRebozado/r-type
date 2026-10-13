#ifndef EDITOR_PLUGINS_CAMERAPLUGINEDITOR_HPP_
#define EDITOR_PLUGINS_CAMERAPLUGINEDITOR_HPP_

#include "PluginEditor.hpp"

namespace Editor {

/**
 * @class CameraPluginEditor
 * @brief Shows and edits the CameraComponent in the Inspector Panel.
 *
 * Position and rotation of the camera are edited through the Position and
 * Rotation editors of the same entity.
 */
class CameraPluginEditor : public IPluginEditor {
  public:
    const char* GetName() const override;
    const char* GetIcon() const override;
    bool OnImGuiRender(Engine::Entity entity, Engine::Registry& registry) override;
    bool HasComponent(Engine::Entity entity, Engine::Registry& registry) override;
    void AddComponent(Engine::Entity entity, Engine::Registry& registry) override;
    void RemoveComponent(Engine::Entity entity, Engine::Registry& registry) override;
};

} // namespace Editor

#endif // EDITOR_PLUGINS_CAMERAPLUGINEDITOR_HPP_
