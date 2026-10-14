#ifndef EDITOR_PANELS_HIERARCHYPANEL_HPP_
#define EDITOR_PANELS_HIERARCHYPANEL_HPP_

/**
 * @file HierarchyPanel.hpp
 * @brief Declares the scene entity hierarchy panel and selection state.
 */

#include "Panels/IEditorPanel.hpp"
#include <string>

namespace Editor {

/**
 * @struct HierarchyPanelSelectedEntity
 * @brief Registry resource storing the currently selected entity.
 */
struct HierarchyPanelSelectedEntity {
    /** @brief Selected entity id, or -1 when nothing is selected. */
    Engine::Entity entity = -1;

    /** @brief Selected entity index in the visible hierarchy list. */
    int index = -1;
};

/**
 * @class HierarchyPanel
 * @brief Displays all registry entities and controls selection/creation.
 */
class HierarchyPanel : public IEditorPanel {
  public:
    HierarchyPanel() = default;

    ~HierarchyPanel() override = default;

    void OnImGuiRender(Engine::Registry& registry, EditorConfig& config) override;

    const std::string& GetName() const override;

  private:
    std::string m_Name = "Hierarchy";
};
} // namespace Editor
#endif // EDITOR_PANELS_HIERARCHYPANEL_HPP_
