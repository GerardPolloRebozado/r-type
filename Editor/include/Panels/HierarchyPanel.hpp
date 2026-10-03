#ifndef EDITOR_PANELS_HIERARCHYPANEL_HPP_
#define EDITOR_PANELS_HIERARCHYPANEL_HPP_

#include "Panels/IEditorPanel.hpp"
#include <string>

namespace Editor {

class HierarchyPanel : public IEditorPanel {
  public:
    HierarchyPanel() = default;
    ~HierarchyPanel() override = default;

    void OnImGuiRender(Engine::Registry* registry, EditorConfig* config) override;
    const std::string& GetName() const override {
        return m_Name;
    }

    Engine::Entity* GetSelectedEntity() {
        if (m_SelectedIndex >= 0 && m_SelectedEntity != -1) {
            return &m_SelectedEntity;
        }
        return nullptr;
    }

  private:
    std::string m_Name = "Hierarchy";
    int m_SelectedIndex = -1;
    Engine::Entity m_SelectedEntity = -1;
};

} // namespace Editor
#endif // EDITOR_PANELS_HIERARCHYPANEL_HPP_
