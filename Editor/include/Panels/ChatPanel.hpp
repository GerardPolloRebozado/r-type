#ifndef EDITOR_PANELS_CHATPANEL_HPP_
#define EDITOR_PANELS_CHATPANEL_HPP_

#include "Author/Commands.hpp"
#include "Chat/OpenRouter.hpp"
#include "Panels/IEditorPanel.hpp"
#include "Resources/WaveSchedule.hpp"
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace Editor {

/**
 * @brief Docked chat that edits the open scene through author tools.
 *
 * The API key and the transcript live only in this panel. They are wiped
 * when the panel is destroyed and are never written to the project.
 */
class ChatPanel : public IEditorPanel {
  public:
    ChatPanel();
    ~ChatPanel() override;

    void OnImGuiRender(Engine::Registry& registry, EditorConfig& config) override;
    const std::string& GetName() const override;

  private:
    struct Line {
        std::string role;
        std::string text;
    };

    void Send(Engine::Registry& registry, EditorConfig& config);
    void StartRequest(const std::string& messagesJson);
    void Poll(Engine::Registry& registry, EditorConfig& config);
    void ApplyCompletion(ChatCompletion completion, Engine::Registry& registry, EditorConfig& config);
    void FinishTurn(Engine::Registry& registry, EditorConfig& config, const std::string& text);
    void Keep();
    void Undo(Engine::Registry& registry, EditorConfig& config);
    void ApplyCredentials();
    void Snapshot(Engine::Registry& registry, EditorConfig& config);
    bool SceneChanged(Engine::Registry& registry, EditorConfig& config) const;
    std::string BuildMessages(Engine::Registry& registry, EditorConfig& config) const;
    std::string RunTool(const ChatToolCall& call, Engine::Registry& registry, EditorConfig& config);

    std::string m_Name = "Chat";
    std::string m_ApiKey;
    std::string m_Model = "openai/gpt-4o-mini";
    std::string m_Url = "https://openrouter.ai/api/v1/chat/completions";
    std::string m_Extra;
    std::string m_DraftKey;
    std::string m_DraftModel = "openai/gpt-4o-mini";
    std::string m_DraftUrl = "https://openrouter.ai/api/v1/chat/completions";
    std::string m_DraftExtra;
    bool m_SettingsEditing = false;
    std::string m_Input;
    std::vector<Line> m_Lines;
    std::vector<std::string> m_History;
    std::string m_Tools;

    bool m_Waiting = false;
    int m_Rounds = 0;
    std::thread m_Thread;
    std::mutex m_Mutex;
    bool m_PendingReady = false;
    ChatCompletion m_Pending;

    bool m_Review = false;
    bool m_HasSnapshot = false;
    std::string m_Entities;
    std::vector<Engine::WaveDefinition> m_WaveSnapshot;
    std::vector<Author::ScriptFileChange> m_Files;
};

} // namespace Editor

#endif // EDITOR_PANELS_CHATPANEL_HPP_
