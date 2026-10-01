#ifndef EDITOR_PANELS_SCENEVIEWPANEL_HPP_
#define EDITOR_PANELS_SCENEVIEWPANEL_HPP_

#include "Panels/IEditorPanel.hpp"
#include <string>

#include <raylib.h>

namespace Editor {

class SceneViewPanel : public IEditorPanel {
  public:
    SceneViewPanel() = default;
    ~SceneViewPanel() override {
        if (m_PreviewTexture.id != 0) {
            UnloadTexture(m_PreviewTexture);
        }
    }

    void OnImGuiRender(Engine::Registry* registry, EditorConfig* config) override;
    const std::string& GetName() const override {
        return m_Name;
    }

  private:
    std::string m_Name = "Scene";
    std::string m_CurrentPreviewPath;
    Texture2D m_PreviewTexture = {0};
};

} // namespace Editor
#endif // EDITOR_PANELS_SCENEVIEWPANEL_HPP_
