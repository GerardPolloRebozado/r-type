#ifndef EDITOR_PANELS_INSPECTORPANEL_HPP_
#define EDITOR_PANELS_INSPECTORPANEL_HPP_

/**
 * @file InspectorPanel.hpp
 * @brief Declares the entity inspector panel and plugin editors.
 */

#include "Panels/IEditorPanel.hpp"
#include "PluginEditor.hpp"
#include <memory>
#include <string>
#include <vector>

namespace Editor {

/**
 * @class InspectorPanel
 * @brief Shows the components of the selected entity through plugin editors.
 */
class InspectorPanel : public IEditorPanel {
  public:
    InspectorPanel();
    ~InspectorPanel() override = default;

    void OnImGuiRender(Engine::Registry& registry, EditorConfig& config) override;
    const std::string& GetName() const override;

    void SetSelectedEntity(Engine::Entity entity);

  private:
    std::string m_Name = "Inspector";
    Engine::Entity m_SelectedEntity = -1;

    // We will hold a list of plugin editors
    std::vector<std::unique_ptr<IPluginEditor>> m_PluginEditors;
};

} // namespace Editor
#endif // EDITOR_PANELS_INSPECTORPANEL_HPP_
