#ifndef EDITOR_PANELS_UIPALETTEPANEL_HPP_
#define EDITOR_PANELS_UIPALETTEPANEL_HPP_

#include "Panels/IEditorPanel.hpp"
#include <string>

namespace Editor {

/**
 * @brief Lists premade UI templates. Drag a row onto the hierarchy to place it.
 */
class UiPalettePanel : public IEditorPanel {
  public:
    UiPalettePanel();
    ~UiPalettePanel() override = default;

    void OnImGuiRender(Engine::Registry& registry, EditorConfig& config) override;
    const std::string& GetName() const override;

    static bool s_IsOpen;

  private:
    std::string m_Name;
};

} // namespace Editor

#endif // EDITOR_PANELS_UIPALETTEPANEL_HPP_
