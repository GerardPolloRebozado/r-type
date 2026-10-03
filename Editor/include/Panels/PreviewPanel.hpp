#ifndef EDITOR_PANELS_PREVIEWPANEL_HPP_
#define EDITOR_PANELS_PREVIEWPANEL_HPP_

#include "Panels/IEditorPanel.hpp"
#include <raylib-cpp.hpp>
#include <string>

namespace Editor {

class PreviewPanel : public IEditorPanel {
  public:
    PreviewPanel() = default;
    ~PreviewPanel() override = default;

    void OnImGuiRender(Engine::Registry* registry, EditorConfig* config) override;
    const std::string& GetName() const override {
        return m_Name;
    }

  private:
    std::string m_Name = "Preview";
    std::string m_CurrentPreviewPath;
    raylib::Texture2D m_PreviewTexture;
    bool m_IsOpen = false;
};

} // namespace Editor
#endif // EDITOR_PANELS_PREVIEWPANEL_HPP_
