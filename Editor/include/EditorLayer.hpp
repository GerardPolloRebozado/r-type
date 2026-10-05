#ifndef EDITOR_EDITORLAYER_HPP_
#define EDITOR_EDITORLAYER_HPP_

#include "ECS/Registry.hpp"
#include "EditorConfig.hpp"
#include "Panels/IEditorPanel.hpp"
#include <memory>
#include <vector>

struct ImFont;

namespace Editor {

extern ImFont* g_IconFont;

class EditorLayer {
  public:
    EditorLayer();
    ~EditorLayer();

    void Init();
    void Shutdown();
    void OnUpdate();

    // Register a new panel to the editor
    void AddPanel(std::unique_ptr<IEditorPanel> panel);

    void SetRegistry(Engine::Registry* registry);

    void SetConfiguration(EditorConfig* config);

  private:
    void BeginDockSpace();
    void EndDockSpace();

    std::vector<std::unique_ptr<IEditorPanel>> m_Panels;
    bool m_FirstTimeLayout = true;
    Engine::Registry* m_Registry = nullptr;
    EditorConfig* m_Configuration = nullptr;
};

} // namespace Editor
#endif // EDITOR_EDITORLAYER_HPP_
