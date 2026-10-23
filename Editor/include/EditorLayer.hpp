#ifndef EDITOR_EDITORLAYER_HPP_
#define EDITOR_EDITORLAYER_HPP_

#include "ECS/Registry.hpp"
#include "EditorConfig.hpp"
#include "Panels/IEditorPanel.hpp"
#include <memory>
#include <vector>

struct ImFont;

namespace Editor {

class ScriptEditorPanel;

extern ImFont* g_IconFont;
extern ImFont* g_CodeFont;

class EditorLayer {
  public:
    EditorLayer();
    ~EditorLayer();

    void Init();
    void Shutdown();
    void OnUpdate();
    void OnRenderGame(Engine::Registry& registry);

    // Register a new panel to the editor
    void AddPanel(std::unique_ptr<IEditorPanel> panel);

    void SetRegistry(Engine::Registry& registry);

    void SetConfiguration(EditorConfig& config);

    // Launch a game process with args, tracked so StopGames can kill it.
    // Returns false if the exe couldn't be found.
    bool LaunchGame(const std::vector<std::string>& args);
    // Kill every game process launched by LaunchGame since the last StopGames.
    void StopGames();

  private:
    void BeginDockSpace();
    void EndDockSpace();

    std::vector<std::unique_ptr<IEditorPanel>> m_Panels;
    bool m_FirstTimeLayout = true;
    Engine::Registry* m_Registry = nullptr;
    std::unique_ptr<EditorConfig> m_Configuration = nullptr;
    ScriptEditorPanel* m_ScriptEditorPanel = nullptr;

#ifdef _WIN32
    // Process handles for launched game windows.
    std::vector<void*> m_GameProcesses;
#else
    // PIDs for launched game processes (started in their own session so
    // StopGames can signal just the game, not the shell).
    std::vector<int> m_GameProcesses;
#endif
};

} // namespace Editor
#endif // EDITOR_EDITORLAYER_HPP_
