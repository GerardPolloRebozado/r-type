#pragma once

#include "Panels/IEditorPanel.hpp"
#include <string>

namespace Editor {

class HierarchyPanel : public IEditorPanel {
  public:
    HierarchyPanel() = default;
    ~HierarchyPanel() override = default;

    void OnImGuiRender() override;
    const std::string& GetName() const override {
        return m_Name;
    }

  private:
    std::string m_Name = "Hierarchy";
};

} // namespace Editor
