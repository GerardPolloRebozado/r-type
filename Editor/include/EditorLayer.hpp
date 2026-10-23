#ifndef EDITOR_EDITORLAYER_HPP_
#define EDITOR_EDITORLAYER_HPP_

#include "ECS/Registry.hpp"
#include "EditorConfig.hpp"
#include "Panels/IEditorPanel.hpp"
#include <memory>
#include <string>
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

    // --- Switching scenes ---

    // What to do once unsaved changes are handled.
    struct SceneAction {
        enum class Kind { None, Open, Create } kind = Kind::None;
        std::string path; // absolute .scene path
    };
    // Which name dialog is open.
    enum class SceneNameDialog { None, NewScene, SaveAs };

    // Starts an action, asking to save first if the scene has unsaved changes.
    void RequestSceneAction(SceneAction action);
    void RunSceneAction(const SceneAction& action);
    // Replaces the registry's entities with the scene at path.
    void OpenScene(const std::string& path);
    // Empties the registry and makes path the active (saved, empty) scene.
    void CreateScene(const std::string& path);
    // Saves the current entities to path and makes it the active scene.
    void SaveSceneAs(const std::string& path);
    // Destroys every entity and resets the selection.
    void ClearScene();

    // Unsaved-changes tracking: JSON of the entities as last loaded/saved.
    std::string CaptureSceneJson();
    bool HasUnsavedChanges();
    void MarkSceneSaved();

    // Every .scene in the project, relative paths, sorted.
    std::vector<std::string> ListProjectScenes() const;
    void OpenSceneNameDialog(SceneNameDialog dialog, const std::string& folder);
    void HandleSceneRequests();
    void DrawSceneMenuItems();
    void DrawSceneDialogs();

    void BeginDockSpace();
    void EndDockSpace();

    std::vector<std::unique_ptr<IEditorPanel>> m_Panels;
    bool m_FirstTimeLayout = true;
    Engine::Registry* m_Registry = nullptr;
    std::unique_ptr<EditorConfig> m_Configuration = nullptr;

    // Scene switching state.
    std::string m_SavedSceneJson;
    // The baseline is taken one frame after a load/save, once editor systems
    // have settled values like prevX/prevY, to avoid false "unsaved" prompts.
    bool m_SavedSceneJsonStale = true;
    SceneAction m_PendingSceneAction;
    bool m_OpenUnsavedPrompt = false;
    SceneNameDialog m_NameDialog = SceneNameDialog::None;
    bool m_OpenNameDialog = false;
    std::string m_NameDialogText;
    std::string m_NameDialogFolder;
    std::string m_NameDialogError;

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
