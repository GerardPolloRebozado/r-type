#ifndef EDITOR_PANELS_SCENEVIEWPANEL_HPP_
#define EDITOR_PANELS_SCENEVIEWPANEL_HPP_

#include "Panels/IEditorPanel.hpp"
#include <raylib-cpp.hpp>
#include <string>

namespace Editor {

class SceneViewPanel : public IEditorPanel {
  public:
    SceneViewPanel();
    ~SceneViewPanel() override;

    void RenderGame(Engine::Registry& registry, EditorConfig& config);
    void OnImGuiRender(Engine::Registry& registry, EditorConfig& config) override;
    const std::string& GetName() const override;

  private:
    std::string m_Name = "Scene";

    enum class GizmoState { None, DraggingX, DraggingY, DraggingBoth, DraggingRotation };
    GizmoState m_GizmoState = GizmoState::None;
    float m_DragOffset[2] = {0.0f, 0.0f};
    float m_InitialDragAngle = 0.0f;
    float m_InitialEntityAngle = 0.0f;

    raylib::RenderTexture2D m_GameTexture;
};

} // namespace Editor
#endif // EDITOR_PANELS_SCENEVIEWPANEL_HPP_
