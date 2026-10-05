#ifndef EDITOR_PANELS_SCENEVIEWPANEL_HPP_
#define EDITOR_PANELS_SCENEVIEWPANEL_HPP_

#include "Panels/IEditorPanel.hpp"
#include <string>

namespace Editor {

class SceneViewPanel : public IEditorPanel {
  public:
    SceneViewPanel() = default;
    ~SceneViewPanel() override = default;

    void OnImGuiRender(Engine::Registry* registry, EditorConfig* config) override;
    const std::string& GetName() const override;

  private:
    std::string m_Name = "Scene";
};

} // namespace Editor
#endif // EDITOR_PANELS_SCENEVIEWPANEL_HPP_
