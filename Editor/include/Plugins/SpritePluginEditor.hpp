#ifndef EDITOR_PLUGINS_SPRITEPLUGINEDITOR_HPP_
#define EDITOR_PLUGINS_SPRITEPLUGINEDITOR_HPP_

#include "PluginEditor.hpp"

namespace Editor {

class SpritePluginEditor : public IPluginEditor {
  public:
    const char* GetName() const override;
    const char* GetIcon() const override;
    bool OnImGuiRender(Engine::Entity entity, Engine::Registry& registry) override;
    bool HasComponent(Engine::Entity entity, Engine::Registry& registry) override;
    void AddComponent(Engine::Entity entity, Engine::Registry& registry) override;
    void RemoveComponent(Engine::Entity entity, Engine::Registry& registry) override;
};

} // namespace Editor

#endif // EDITOR_PLUGINS_SPRITEPLUGINEDITOR_HPP_
