#ifndef EDITOR_RENDERING_SCENERENDERER_HPP_
#define EDITOR_RENDERING_SCENERENDERER_HPP_

#include "ECS/Registry.hpp"
#include "EditorConfig.hpp"
#include <raylib-cpp.hpp>

namespace Editor {

/**
 * @class SceneRenderer
 * @brief Dedicated rendering pipeline for the Editor viewport.
 *
 * Separates framebuffer management, ECS render systems dispatch,
 * and debug overlay passes (such as collider outlines) from ImGui UI code.
 */
class SceneRenderer {
  public:
    SceneRenderer() = default;
    ~SceneRenderer();

    /**
     * @brief Executes the complete viewport render pipeline into an off-screen render texture.
     */
    void Render(Engine::Registry& registry, const EditorConfig& config);

    /**
     * @brief Returns the rendered texture to be displayed by ImGui panels.
     */
    const raylib::RenderTexture2D& GetTargetTexture() const {
        return m_GameTexture;
    }

  private:
    void RenderSystems(Engine::Registry& registry);
    void RenderColliderOverlays(Engine::Registry& registry);

    raylib::RenderTexture2D m_GameTexture;
};

} // namespace Editor

#endif // EDITOR_RENDERING_SCENERENDERER_HPP_
