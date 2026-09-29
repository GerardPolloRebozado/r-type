#pragma once

#include "Panels/IEditorPanel.hpp"
#include <string>

namespace Editor {

class InspectorPanel : public IEditorPanel {
  public:
    InspectorPanel() = default;
    ~InspectorPanel() override = default;

    void OnImGuiRender() override;
    const std::string& GetName() const override {
        return m_Name;
    }

  private:
    std::string m_Name = "Inspector";
};

} // namespace Editor
