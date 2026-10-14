#ifndef EDITOR_PANELS_SCENEVIEWPANEL_HPP_
#define EDITOR_PANELS_SCENEVIEWPANEL_HPP_

/**
 * @file SceneViewPanel.hpp
 * @brief Declares the interactive scene viewport and gizmo handling.
 */

#include "Panels/IEditorPanel.hpp"
#include "Rendering/SceneRenderer.hpp"
#include <raylib.h>
#include <string>

namespace Editor {

/**
 * @class SceneViewPanel
 * @brief Renders the scene and handles translate/rotate gizmos for the selected entity.
 */
class SceneViewPanel : public IEditorPanel {
  public:
    SceneViewPanel() = default;
    ~SceneViewPanel() override = default;

    /**
     * @brief Renders the scene world into the panel render target.
     * @param registry Registry being edited.
     * @param config Editor configuration used for viewport size.
     */
    void RenderGame(Engine::Registry& registry, EditorConfig& config);

    /**
     * @brief Draws the scene viewport and gizmos.
     */
    void OnImGuiRender(Engine::Registry& registry, EditorConfig& config) override;

    /**
     * @brief Returns the panel name used for docking.
     */
    const std::string& GetName() const override;

  private:
    std::string m_Name = "Scene";

    /** @brief Interaction state of the scene gizmo. */
    enum class GizmoState { None, DraggingX, DraggingY, DraggingBoth, DraggingRotation };

    /** @brief Current gizmo interaction state. */
    GizmoState m_GizmoState = GizmoState::None;

    /** @brief Translation offset recorded when a drag starts. */
    float m_DragOffset[2] = {0.0f, 0.0f};

    /** @brief Pointer angle recorded when a rotation drag starts. */
    float m_InitialDragAngle = 0.0f;

    /** @brief Entity angle recorded when a rotation drag starts. */
    float m_InitialEntityAngle = 0.0f;

    // Camera frozen when a drag starts, so dragging the camera entity itself does not feed back
    Camera2D m_DragCamera{{0.0f, 0.0f}, {0.0f, 0.0f}, 0.0f, 1.0f};

    /** @brief Scene renderer used to draw the game world. */
    SceneRenderer m_Renderer;
};

} // namespace Editor
#endif // EDITOR_PANELS_SCENEVIEWPANEL_HPP_