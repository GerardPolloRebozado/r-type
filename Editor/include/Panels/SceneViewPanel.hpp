#ifndef EDITOR_PANELS_SCENEVIEWPANEL_HPP_
#define EDITOR_PANELS_SCENEVIEWPANEL_HPP_

#include "Panels/IEditorPanel.hpp"
#include "Rendering/SceneRenderer.hpp"
#include <raylib.h>
#include <string>

namespace Editor {

class SceneViewPanel : public IEditorPanel {
  public:
    SceneViewPanel() = default;
    ~SceneViewPanel() override = default;

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
    // Camera frozen when a drag starts, so dragging the camera entity itself does not feed back
    Camera2D m_DragCamera{{0.0f, 0.0f}, {0.0f, 0.0f}, 0.0f, 1.0f};
    Camera2D m_EditorCamera{{0.0f, 0.0f}, {0.0f, 0.0f}, 0.0f, 1.0f};
    bool m_EditorCameraInitialized = false;
    SceneRenderer m_Renderer;
};

} // namespace Editor
#endif // EDITOR_PANELS_SCENEVIEWPANEL_HPP_
