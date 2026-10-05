#ifndef EDITOR_PANELS_HIERARCHYPANEL_HPP_
#define EDITOR_PANELS_HIERARCHYPANEL_HPP_

#include "Panels/IEditorPanel.hpp"
#include <string>

namespace Editor {
struct HierarchyPanelSelectedEntity {
    Engine::Entity entity = -1;
    int index = -1;
};

class HierarchyPanel : public IEditorPanel {
  public:
    HierarchyPanel() = default;

    ~HierarchyPanel() override = default;

    void OnImGuiRender(Engine::Registry* registry, EditorConfig* config) override;

    const std::string& GetName() const override;

  private:
    std::string m_Name = "Hierarchy";
};
} // namespace Editor
#endif // EDITOR_PANELS_HIERARCHYPANEL_HPP_
