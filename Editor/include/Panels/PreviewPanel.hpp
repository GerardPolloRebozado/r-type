#ifndef EDITOR_PANELS_PREVIEWPANEL_HPP_
#define EDITOR_PANELS_PREVIEWPANEL_HPP_

#include "Panels/IEditorPanel.hpp"
#include <raylib.h>
#include <string>

namespace Editor {

class PreviewPanel : public IEditorPanel {
  public:
    PreviewPanel() = default;
    ~PreviewPanel() override {
        if (m_PreviewTexture.id != 0) {
            UnloadTexture(m_PreviewTexture);
        }
    }

    void OnImGuiRender(Engine::Registry* registry, EditorConfig* config) override;
    const std::string& GetName() const override {
        return m_Name;
    }

  private:
    std::string m_Name = "Preview";
    std::string m_CurrentPreviewPath;
    Texture2D m_PreviewTexture = {0};
    bool m_IsOpen = false;
};

} // namespace Editor
#endif // EDITOR_PANELS_PREVIEWPANEL_HPP_
