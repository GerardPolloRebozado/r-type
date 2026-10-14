#ifndef EDITOR_PANELS_CONSOLEPANEL_HPP_
#define EDITOR_PANELS_CONSOLEPANEL_HPP_

/**
 * @file ConsolePanel.hpp
 * @brief Declares the editor log console panel.
 */

#include "Logging/LogBuffer.hpp"
#include "Panels/IEditorPanel.hpp"
#include <string>
#include <vector>

namespace Editor {

/**
 * @class ConsolePanel
 * @brief Displays buffered engine log entries inside the editor.
 */
class ConsolePanel : public IEditorPanel {
  public:
    ConsolePanel() = default;
    ~ConsolePanel() override = default;

    void OnImGuiRender(Engine::Registry& registry, EditorConfig& config) override;
    const std::string& GetName() const override;

  private:
    std::string m_Name = "Console";
    std::vector<Engine::LogEntry> m_Entries;
    // Pinned to the latest line until the user scrolls up. Resumes at the bottom.
    bool m_ScrollToBottom = true;
};

} // namespace Editor
#endif // EDITOR_PANELS_CONSOLEPANEL_HPP_
