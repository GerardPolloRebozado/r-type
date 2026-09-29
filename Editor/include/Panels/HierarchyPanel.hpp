#pragma once

#include "Panels/IEditorPanel.hpp"
#include <string>

namespace Editor {

class HierarchyPanel : public IEditorPanel {
  public:
    HierarchyPanel() = default;
    ~HierarchyPanel() override = default;

    void OnImGuiRender(Engine::Registry* registry) override;
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
