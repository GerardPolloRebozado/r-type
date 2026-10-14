#ifndef EDITOR_PANELS_PREVIEWPANEL_HPP_
#define EDITOR_PANELS_PREVIEWPANEL_HPP_

/**
 * @file PreviewPanel.hpp
 * @brief Declares the asset preview panel.
 */

#include "Panels/IEditorPanel.hpp"
#include <raylib-cpp.hpp>
#include <string>

namespace Editor {

/**
 * @class PreviewPanel
 * @brief Displays a preview of the currently selected asset (textures, sounds, ...).
 */
class PreviewPanel : public IEditorPanel {
  public:
    PreviewPanel() = default;
    ~PreviewPanel() override = default;

    /**
     * @brief Renders the asset preview window.
     */
    void OnImGuiRender(Engine::Registry& registry, EditorConfig& config) override;

    /**
     * @brief Returns the panel name used for docking.
     */
    const std::string& GetName() const override;

  private:
    std::string m_Name = "Preview";
    std::string m_CurrentPreviewPath;
    raylib::Texture2D m_PreviewTexture;
    bool m_IsOpen = false;
};

} // namespace Editor
#endif // EDITOR_PANELS_PREVIEWPANEL_HPP_