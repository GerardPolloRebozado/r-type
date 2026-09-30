#ifndef EDITOR_PANELS_IEDITORPANEL_HPP_
#define EDITOR_PANELS_IEDITORPANEL_HPP_


#include "ECS/Registry.hpp"
#include <string>

namespace Editor {

class IEditorPanel {
  public:
    virtual ~IEditorPanel() = default;

    // Called every frame to render the panel's ImGui window
    virtual void OnImGuiRender(Engine::Registry* registry) = 0;

    // Get the name of the panel for docking/identification
    virtual const std::string& GetName() const = 0;
};

} // namespace Editor
#endif // EDITOR_PANELS_IEDITORPANEL_HPP_
