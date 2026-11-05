#ifndef EDITOR_PANELS_SCENEVIEWPANEL_HPP_
#define EDITOR_PANELS_SCENEVIEWPANEL_HPP_

#include "Panels/IEditorPanel.hpp"
#include "Rendering/SceneRenderer.hpp"
#include <array>
#include <optional>
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
    /**
     * @brief Frames the editor camera on the game camera's viewport, once.
     *
     * The game camera's visible area fills kGameViewFillRatio of the scene view,
     * so both what the game will render and its surroundings are visible.
     * No-op after the first success, and retries while sizes are unknown.
     */
    void InitEditorCamera(Engine::Registry& registry, float viewWidth, float viewHeight);

    /**
     * @brief Corners of the region the game camera renders, in render-target pixels.
     *
     * Maps the game canvas corners through the game camera into world space and then
     * through the editor camera into the render target.
     */
    std::array<Vector2, 4> GameViewportInTarget(Engine::Registry& registry, int canvasWidth,
                                                int canvasHeight) const;

    /**
     * @brief Camera mapping game canvas pixels into the region the game camera renders.
     *
     * Used for the screen-space (HUD) pass so UI is previewed where the game shows it.
     * Null when there is no canvas to map.
     */
    std::optional<Camera2D> GameScreenCamera(Engine::Registry& registry) const;

    /**
     * @brief Page drawn inside the game view.
     *
     * Moves to the page of the selected entity when the selection or its page
     * changes. Otherwise the page picked in the Page combo, else the start page.
     */
    std::string PickPreviewPage(Engine::Registry& registry);

    std::string m_Name = "Scene";

    std::string m_PreviewPage;
    // Selection and its page seen last frame, so the Page combo is not overridden every frame.
    Engine::Entity m_PreviewSelection = static_cast<Engine::Entity>(-1);
    std::string m_PreviewSelectionPage;

    enum class GizmoState { None, DraggingX, DraggingY, DraggingBoth, DraggingRotation };
    GizmoState m_GizmoState = GizmoState::None;
    float m_DragOffset[2] = {0.0f, 0.0f};
    float m_InitialDragAngle = 0.0f;
    float m_InitialEntityAngle = 0.0f;
    // Camera frozen when a drag starts, so dragging the camera entity itself does not feed back
    Camera2D m_DragCamera{{0.0f, 0.0f}, {0.0f, 0.0f}, 0.0f, 1.0f};
    Camera2D m_EditorCamera{{0.0f, 0.0f}, {0.0f, 0.0f}, 0.0f, 1.0f};
    bool m_EditorCameraInitialized = false;
    // Size of the scene tab, in pixels, as reported by the last frame. The scene is
    // rendered before the UI, so framing and the render target follow it one frame late.
    Vector2 m_ViewportSize{0.0f, 0.0f};
    SceneRenderer m_Renderer;
};

} // namespace Editor
#endif // EDITOR_PANELS_SCENEVIEWPANEL_HPP_
