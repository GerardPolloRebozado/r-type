#ifndef EDITOR_PLUGINS_UIPLUGINEDITOR_HPP_
#define EDITOR_PLUGINS_UIPLUGINEDITOR_HPP_

#include "PluginEditor.hpp"
#include <string>

namespace Editor {

/**
 * @brief Inspector for a premade UI template or a custom UI script.
 */
class UiPluginEditor : public IPluginEditor {
  public:
    const char* GetName() const override;
    const char* GetIcon() const override;

    bool OnImGuiRender(Engine::Entity entity, Engine::Registry& registry) override;
    bool HasComponent(Engine::Entity entity, Engine::Registry& registry) override;
    void AddComponent(Engine::Entity entity, Engine::Registry& registry) override;
    void RemoveComponent(Engine::Entity entity, Engine::Registry& registry) override;

  private:
    /**
     * @brief Page name field of a screen. The rename runs when the edit is committed,
     * so buttons that point at the old name follow it once.
     */
    void DrawPageName(Engine::Registry& registry, Engine::Entity entity);

    std::string m_PageBuffer;
    Engine::Entity m_PageEntity = static_cast<Engine::Entity>(-1);
    bool m_PageEditing = false;
};

/**
 * @brief Draws the Add UI submenu when this editor is the UI one.
 * @return True when the generic "Add UI" item should be skipped.
 */
bool TryDrawUiAddMenu(IPluginEditor& editor, Engine::Entity entity, Engine::Registry& registry);

} // namespace Editor

#endif // EDITOR_PLUGINS_UIPLUGINEDITOR_HPP_
