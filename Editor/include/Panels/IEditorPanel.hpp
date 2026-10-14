#ifndef EDITOR_PANELS_IEDITORPANEL_HPP_
#define EDITOR_PANELS_IEDITORPANEL_HPP_

/**
 * @file IEditorPanel.hpp
 * @brief Interface implemented by every editor panel.
 */

#include "Core/Application.hpp"
#include "ECS/Registry.hpp"
#include <string>

namespace Editor {
struct EditorConfig;

/**
 * @class IEditorPanel
 * @brief Interface for dockable ImGui panels owned by the editor.
 */
class IEditorPanel {
  public:
    virtual ~IEditorPanel() = default;

    /**
     * @brief Called every frame to render the panel.
     * @param registry Registry being edited/rendered.
     * @param config Current editor/project configuration.
     */
    virtual void OnImGuiRender(Engine::Registry& registry, EditorConfig& config) = 0;

    /**
     * @brief Gets the panel name used for docking and identification.
     */
    virtual const std::string& GetName() const = 0;
};

} // namespace Editor
#endif // EDITOR_PANELS_IEDITORPANEL_HPP_