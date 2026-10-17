#ifndef EDITOR_RENDERING_SCENERENDERER_HPP_
#define EDITOR_RENDERING_SCENERENDERER_HPP_

#include "Core/VirtualResolution.hpp"
#include "ECS/Registry.hpp"
#include "EditorConfig.hpp"
#include <optional>
#include <raylib.h>

namespace Editor {

/**
 * @class SceneRenderer
 * @brief Dedicated rendering pipeline for the Editor viewport.
 *
 * Separates framebuffer management, ECS render systems dispatch,
 * and debug overlay passes (such as collider outlines) from ImGui UI code.
 *
 * The scene is drawn into a render texture sized to the panel, not to the game
 * canvas: the scene view is an infinite viewport that fills its tab entirely and
 * can be panned and zoomed freely. The game canvas stays the engine
 * VirtualResolution resource, which is what the game camera renders and what the
 * scene view outlines.
 */
class SceneRenderer {
  public:
    SceneRenderer() = default;
    ~SceneRenderer();

    SceneRenderer(const SceneRenderer&) = delete;
    SceneRenderer& operator=(const SceneRenderer&) = delete;
    SceneRenderer(SceneRenderer&&) = delete;
    SceneRenderer& operator=(SceneRenderer&&) = delete;

    /**
     * @brief Executes the complete viewport render pipeline.
     * @param viewportSize Size of the surface to draw into, in pixels.
     * @param editorCamera Camera the world pass is drawn with.
     * @param screenCamera Optional camera for the screen-space (HUD) pass, so UI is
     *        previewed where the game would show it. Omitted => plain pixels.
     */
    void Render(Engine::Registry& registry, const EditorConfig& config, Vector2 viewportSize,
                std::optional<Camera2D> editorCamera = std::nullopt,
                std::optional<Camera2D> screenCamera = std::nullopt);

    /**
     * @brief Returns the rendered texture to be displayed by ImGui panels.
     */
    const RenderTexture2D& GetTargetTexture() const {
        return m_Target;
    }

  private:
    /** Recreates the render texture when the viewport size changes. */
    void EnsureTarget(int width, int height);
    /** Releases the GPU render texture. Safe to call more than once. */
    void UnloadTarget();

    void RenderSystems(Engine::Registry& registry, std::optional<Camera2D> editorCamera,
                       std::optional<Camera2D> screenCamera);
    void RenderColliderOverlays(Engine::Registry& registry);

    RenderTexture2D m_Target{};
    int m_Width = 0;
    int m_Height = 0;
};

} // namespace Editor

#endif // EDITOR_RENDERING_SCENERENDERER_HPP_
