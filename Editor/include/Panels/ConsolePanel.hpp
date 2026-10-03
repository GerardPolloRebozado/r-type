#ifndef EDITOR_PANELS_CONSOLEPANEL_HPP_
#define EDITOR_PANELS_CONSOLEPANEL_HPP_

#include "Panels/IEditorPanel.hpp"
#include <string>

namespace Editor {

class ConsolePanel : public IEditorPanel {
  public:
    ConsolePanel() = default;
    ~ConsolePanel() override = default;

    void OnImGuiRender(Engine::Registry* registry, EditorConfig* config) override;
    const std::string& GetName() const override {
        return m_Name;
    }

  private:
    std::string m_Name = "Console";
};

} // namespace Editor
#endif // EDITOR_PANELS_CONSOLEPANEL_HPP_
