#pragma once

#include "Panels/IEditorPanel.hpp"
#include <string>

namespace Editor {

class ConsolePanel : public IEditorPanel {
  public:
    ConsolePanel() = default;
    ~ConsolePanel() override = default;

    void OnImGuiRender() override;
    const std::string& GetName() const override {
        return m_Name;
    }

  private:
    std::string m_Name = "Console";
};

} // namespace Editor
