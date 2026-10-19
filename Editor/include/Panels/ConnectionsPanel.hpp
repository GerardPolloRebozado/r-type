#ifndef EDITOR_PANELS_CONNECTIONSPANEL_HPP_
#define EDITOR_PANELS_CONNECTIONSPANEL_HPP_

#include "Panels/HierarchyPanel.hpp"
#include "Panels/IEditorPanel.hpp"
#include "Plugins/ConnectionsPluginEditor.hpp"
#include <string>

namespace Editor {

/**
 * @class ConnectionsPanel
 * @brief Godot-style docked Connections tab: signal connections of the
 *        currently selected entity, one tab per emitting plugin.
 *
 * Shares its rendering with ConnectionsPluginEditor::RenderTabs so the
 * standalone panel and the Inspector section can never drift apart.
 */
class ConnectionsPanel : public IEditorPanel {
  public:
    ConnectionsPanel() = default;
    ~ConnectionsPanel() override = default;

    void OnImGuiRender(Engine::Registry& registry, EditorConfig& config) override;
    const std::string& GetName() const override;

  private:
    std::string m_Name = "Connections";
    ConnectionsPluginEditor m_Editor;
};

} // namespace Editor
#endif // EDITOR_PANELS_CONNECTIONSPANEL_HPP_
