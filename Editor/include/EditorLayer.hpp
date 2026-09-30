#ifndef EDITOR_EDITORLAYER_HPP_
#define EDITOR_EDITORLAYER_HPP_


#include "ECS/Registry.hpp"
#include "Panels/IEditorPanel.hpp"
#include <memory>
#include <vector>

namespace Editor {

class EditorLayer {
  public:
    EditorLayer();
    ~EditorLayer();

    void Init();
    void Shutdown();
    void OnUpdate();

    // Register a new panel to the editor
    void AddPanel(std::unique_ptr<IEditorPanel> panel);

    void SetRegistry(Engine::Registry* registry) {
        m_Registry = registry;
    }

  private:
    void BeginDockSpace();
    void EndDockSpace();

    std::vector<std::unique_ptr<IEditorPanel>> m_Panels;
    bool m_FirstTimeLayout = true;
    Engine::Registry* m_Registry = nullptr;
};

} // namespace Editor
#endif // EDITOR_EDITORLAYER_HPP_
