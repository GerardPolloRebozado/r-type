#pragma once

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

  private:
    void BeginDockSpace();
    void EndDockSpace();

    std::vector<std::unique_ptr<IEditorPanel>> m_Panels;
    bool m_FirstTimeLayout = true;
};

} // namespace Editor
