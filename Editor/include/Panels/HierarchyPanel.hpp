#ifndef EDITOR_PANELS_HIERARCHYPANEL_HPP_
#define EDITOR_PANELS_HIERARCHYPANEL_HPP_

#include "Panels/IEditorPanel.hpp"
#include <string>
#include <unordered_set>

namespace Editor {
struct HierarchyPanelSelectedEntity {
    Engine::Entity entity = -1;
    int index = -1;
};

class HierarchyPanel : public IEditorPanel {
  public:
    HierarchyPanel() = default;

    ~HierarchyPanel() override = default;

    void OnImGuiRender(Engine::Registry& registry, EditorConfig& config) override;

    const std::string& GetName() const override;

  private:
    std::string m_Name = "Hierarchy";

    void RenderEntityNode(Engine::Registry& registry, Engine::Entity entity,
                          HierarchyPanelSelectedEntity& selected,
                          std::unordered_set<Engine::Entity>& visited, int depth);
};
} // namespace Editor
#endif // EDITOR_PANELS_HIERARCHYPANEL_HPP_
