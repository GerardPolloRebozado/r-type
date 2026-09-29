#pragma once

#include "ComponentEditor.hpp"
#include "Panels/IEditorPanel.hpp"
#include <memory>
#include <string>
#include <vector>

namespace Editor {

class InspectorPanel : public IEditorPanel {
  public:
    InspectorPanel();
    ~InspectorPanel() override = default;

    void OnImGuiRender(Engine::Registry* registry) override;
    const std::string& GetName() const override {
        return m_Name;
    }

    void SetSelectedEntity(Engine::Entity* entity) {
        m_SelectedEntity = (entity ? *entity : -1);
    }

  private:
    std::string m_Name = "Inspector";
    Engine::Entity m_SelectedEntity = -1;

    // We will hold a list of component editors
    std::vector<std::unique_ptr<IComponentEditor>> m_ComponentEditors;
};

} // namespace Editor
