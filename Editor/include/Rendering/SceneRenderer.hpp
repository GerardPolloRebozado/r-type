#ifndef EDITOR_RENDERING_SCENERENDERER_HPP_
#define EDITOR_RENDERING_SCENERENDERER_HPP_

#include "Core/VirtualResolution.hpp"
#include "ECS/Registry.hpp"
#include "EditorConfig.hpp"

namespace Editor {

/**
 * @class SceneRenderer
 * @brief Dedicated rendering pipeline for the Editor viewport.
 *
 * Separates framebuffer management, ECS render systems dispatch,
 * and debug overlay passes (such as collider outlines) from ImGui UI code.
 * The game canvas is the engine VirtualResolution resource.
 */
class SceneRenderer {
  public:
    SceneRenderer() = default;

    /**
     * @brief Executes the complete viewport render pipeline into the virtual canvas.
     */
    void Render(Engine::Registry& registry, const EditorConfig& config);

    /**
     * @brief Returns the rendered texture to be displayed by ImGui panels.
     */
    const RenderTexture2D& GetTargetTexture(Engine::Registry& registry) const {
        return registry.GetResource<Engine::VirtualResolution>().GetTexture();
    }

  private:
    void RenderSystems(Engine::Registry& registry);
    void RenderColliderOverlays(Engine::Registry& registry);
};

} // namespace Editor

#endif // EDITOR_RENDERING_SCENERENDERER_HPP_
