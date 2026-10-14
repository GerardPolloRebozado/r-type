#ifndef EDITOR_EDITORLAYER_HPP_
#define EDITOR_EDITORLAYER_HPP_

/**
 * @file EditorLayer.hpp
 * @brief Declares the top-level layer that owns the editor UI and panels.
 */

#include "ECS/Registry.hpp"
#include "EditorConfig.hpp"
#include "Panels/IEditorPanel.hpp"
#include <memory>
#include <vector>

struct ImFont;

namespace Editor {

extern ImFont* g_IconFont;

/**
 * @class EditorLayer
 * @brief Coordinates the editor lifecycle, panels, registry access and rendering.
 */
class EditorLayer {
  public:
    EditorLayer();
    ~EditorLayer();

    /**
     * @brief Creates the default panels and prepares the workspace layout.
     */
    void Init();

    /**
     * @brief Shuts the editor down and releases panel resources.
     */
    void Shutdown();

    /**
     * @brief Per-frame editor UI callback.
     */
    void OnUpdate();

    /**
     * @brief Per-frame game/render callback used by the preview.
     * @param registry Registry being rendered.
     */
    void OnRenderGame(Engine::Registry& registry);

    /**
     * @brief Adds a new panel to the editor.
     * @param panel Panel instance to own and render.
     */
    void AddPanel(std::unique_ptr<IEditorPanel> panel);

    /**
     * @brief Sets the ECS registry the editor operates on.
     * @param registry Registry pointer.
     */
    void SetRegistry(Engine::Registry* registry);

    /**
     * @brief Sets the current project configuration.
     * @param config Project configuration to copy.
     */
    void SetConfiguration(EditorConfig& config);

  private:
    /**
     * @brief Begins the editor dock space frame.
     */
    void BeginDockSpace();

    /**
     * @brief Ends the editor dock space frame.
     */
    void EndDockSpace();

    std::vector<std::unique_ptr<IEditorPanel>> m_Panels;
    bool m_FirstTimeLayout = true;
    Engine::Registry* m_Registry = nullptr;
    std::unique_ptr<EditorConfig> m_Configuration = nullptr;
};

} // namespace Editor
#endif // EDITOR_EDITORLAYER_HPP_