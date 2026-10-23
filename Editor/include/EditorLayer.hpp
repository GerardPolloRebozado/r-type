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
    // Writes the registry to the active scene file. Returns false if there is
    // nothing to save to.
    bool SaveActiveScene();
    // Active scene path relative to the project folder (the game's working
    // directory), e.g. "test.scene" or "levels/Level1.scene".
    std::string ActiveSceneRelativePath() const;

    void BeginDockSpace();
    void EndDockSpace();

    std::vector<std::unique_ptr<IEditorPanel>> m_Panels;
    bool m_FirstTimeLayout = true;
    Engine::Registry* m_Registry = nullptr;
    std::unique_ptr<EditorConfig> m_Configuration = nullptr;

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
