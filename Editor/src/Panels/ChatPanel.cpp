#include "Panels/ChatPanel.hpp"
#include "Author/Protocol.hpp"
#include "Author/Session.hpp"
#include "EditorConfig.hpp"
#include "Logging/Logger.hpp"
#include <algorithm>
#include <functional>
#include "GlazeSerializer.hpp"
#include "SceneSerializer.hpp"
#include "Panels/HierarchyPanel.hpp"
#include <cctype>
#include <extras/IconsFontAwesome6.h>
#include <filesystem>
#include <fstream>
#include <glaze/glaze.hpp>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace Editor {
namespace {

constexpr int kMaxToolRounds = 8;
constexpr std::size_t kOutlineLimit = 12000;

bool FirstFileBackup(std::vector<std::string>& seen, const std::string& path)
{
    std::filesystem::path file = path;
    std::string key = file.lexically_normal().generic_string();
    std::size_t index = 0;

    for (index = 0; index < seen.size(); ++index) {
        if (seen[index] == key) {
            return false;
        }
    }
    seen.push_back(key);
    return true;
}

std::string WavesText(const std::vector<Engine::WaveDefinition>& waves)
{
    std::string text;

    for (const Engine::WaveDefinition& wave : waves) {
        text += wave.name;
        text += '|';
        text += wave.prefabPath;
        text += '|';
        text += std::to_string(wave.triggerX);
        text += '|';
        text += std::to_string(wave.count);
        text += '|';
        text += std::to_string(wave.spacing);
        text += '|';
        text += std::to_string(wave.baseY);
        text += '|';
        text += std::to_string(wave.spawnOffsetX);
        text += '|';
        text += wave.attachParent ? "1" : "0";
        text += '|';
        text += wave.groupScriptPath;
        text += '|';
        text += std::to_string(static_cast<int>(wave.formation));
        text += '\n';
    }
    return text;
}

std::string CaptureEntities(Engine::Registry& registry)
{
    Serialization::GlazeSerializer serializer;
    return serializer.SerializeToString(serializer.CaptureScene(registry));
}

void RestoreEntities(Engine::Registry& registry, const std::string& json)
{
    Serialization::GlazeSerializer serializer;
    Serialization::GlazeSerializer::SceneSnapshot snapshot;

    if (!serializer.DeserializeFromString(json, snapshot)) {
        return;
    }
    registry.ClearEntities();
    serializer.RestoreScene(registry, snapshot);
}

std::string ShortText(const std::string& text)
{
    if (text.size() <= 400) {
        return text;
    }
    return text.substr(0, 400) + "...";
}

std::string Trim(const std::string& value)
{
    std::size_t begin = 0;
    std::size_t end = value.size();

    while (begin < end && std::isspace(static_cast<unsigned char>(value[begin])) != 0) {
        ++begin;
    }
    while (end > begin && std::isspace(static_cast<unsigned char>(value[end - 1])) != 0) {
        --end;
    }
    return value.substr(begin, end - begin);
}

std::string NormalizeAsk(const std::string& value)
{
    std::string text = Trim(value);
    std::string out;
    std::size_t index = 0;

    out.reserve(text.size());
    for (index = 0; index < text.size(); ++index) {
        unsigned char letter = static_cast<unsigned char>(text[index]);
        if (letter == '.' || letter == '!' || letter == '?') {
            continue;
        }
        if (letter < 128) {
            out.push_back(static_cast<char>(std::tolower(letter)));
        } else {
            out.push_back(static_cast<char>(letter));
        }
    }
    return Trim(out);
}

bool IsKeepRequest(const std::string& value)
{
    std::string text = NormalizeAsk(value);

    return text == "keep" || text == "keep it" || text == "keep them" ||
           text == "keep the changes" || text == "keep the edit" || text == "keep changes" ||
           text == "save" || text == "save it" || text == "save them" ||
           text == "save the changes" || text == "save the scene" || text == "save changes" ||
           text == "guardar" || text == "guarda" || text == "guarda los cambios" ||
           text == "guardar los cambios" || text == "quedate con los cambios" ||
           text == "quédate con los cambios";
}

std::string CompactKey(const std::string& value)
{
    std::string key;
    std::size_t index = 0;

    key.reserve(value.size());
    for (index = 0; index < value.size(); ++index) {
        if (std::isspace(static_cast<unsigned char>(value[index])) == 0) {
            key.push_back(value[index]);
        }
    }
    return key;
}

} // namespace

ChatPanel::ChatPanel()
{
    m_Tools = OpenAiToolsJson(Author::ToolCatalogJson());
}

ChatPanel::~ChatPanel()
{
    if (m_Thread.joinable()) {
        m_Thread.join();
    }
    std::fill(m_ApiKey.begin(), m_ApiKey.end(), '\0');
    std::fill(m_DraftKey.begin(), m_DraftKey.end(), '\0');
    m_ApiKey.clear();
    m_DraftKey.clear();
    m_History.clear();
    m_Lines.clear();
    m_Extra.clear();
}

const std::string& ChatPanel::GetName() const
{
    return m_Name;
}

void ChatPanel::Snapshot(Engine::Registry& registry, EditorConfig& config)
{
    m_Entities = CaptureEntities(registry);
    m_WaveSnapshot = config.m_Waves;
    m_Files.clear();
    m_HasSnapshot = true;
    m_Review = false;
}

bool ChatPanel::SceneChanged(Engine::Registry& registry, EditorConfig& config) const
{
    if (!m_HasSnapshot) {
        return false;
    }
    return CaptureEntities(registry) != m_Entities ||
           WavesText(config.m_Waves) != WavesText(m_WaveSnapshot) || !m_Files.empty();
}

void ChatPanel::ClearSnapshot()
{
    m_HasSnapshot = false;
    m_Entities.clear();
    m_WaveSnapshot.clear();
    m_Files.clear();
    m_Review = false;
}

bool ChatPanel::SaveScene(Engine::Registry& registry, EditorConfig& config)
{
    Author::Session session;
    std::string error;

    error = session.Attach(registry, config, config.m_ProjectFolder, config.m_ActiveScenePath);
    if (error.empty()) {
        error = session.Save();
    }
    if (!error.empty()) {
        m_Lines.push_back({"error", "Could not save the scene: " + error});
        log_user_error("[Chat] Could not save the scene: " + error);
        return false;
    }
    config.m_RequestMarkSaved = true;
    log_info("[Chat] Saved scene " + config.m_ActiveScenePath);
    return true;
}

bool ChatPanel::AcceptEdit(Engine::Registry& registry, EditorConfig& config)
{
    bool changed = SceneChanged(registry, config);

    if (changed && !SaveScene(registry, config)) {
        return false;
    }
    ClearSnapshot();
    return true;
}

void ChatPanel::Undo(Engine::Registry& registry, EditorConfig& config)
{
    std::error_code error;
    std::vector<std::string> restored;

    if (!m_HasSnapshot) {
        return;
    }
    RestoreEntities(registry, m_Entities);
    config.m_Waves = m_WaveSnapshot;
    if (registry.HasResource<Engine::WaveSchedule>()) {
        registry.GetResource<Engine::WaveSchedule>().waves = m_WaveSnapshot;
    }
    for (const Author::ScriptFileChange& file : m_Files) {
        if (!FirstFileBackup(restored, file.path)) {
            continue;
        }
        if (!file.existed) {
            std::filesystem::remove(file.path, error);
            continue;
        }
        std::ofstream out(file.path, std::ios::binary | std::ios::trunc);
        if (out) {
            out << file.previous;
        }
    }
    if (registry.HasResource<HierarchyPanelSelectedEntity>()) {
        registry.GetResource<HierarchyPanelSelectedEntity>().entity =
            static_cast<Engine::Entity>(-1);
    }
    if (!SaveScene(registry, config)) {
        Serialization::SceneSerializer serializer;
        std::optional<std::reference_wrapper<Serialization::SceneConfig>> sceneConfig(config);

        serializer.Deserialize(config.m_ActiveScenePath, registry, sceneConfig);
        m_Lines.push_back(
            {"info", "Could not save the undo. Reloaded the scene from the file."});
        return;
    }
    m_Lines.push_back({"info", "Undid the last chat edit and saved the scene."});
    ClearSnapshot();
}

std::string ChatPanel::BuildMessages(Engine::Registry& registry, EditorConfig& config) const
{
    Author::AuthorRequest request;
    std::vector<Author::ScriptFileChange> ignored;
    glz::generic outline;
    std::string outlineJson;
    std::string prompt;
    std::string messages = "[";
    bool first = true;

    request.apiVersion = Author::kApiVersion;
    request.command = "scene_outline";
    request.projectFolder = config.m_ProjectFolder;
    request.scenePath = config.m_ActiveScenePath;
    request.engineRoot = RTYPE_SOURCE_DIR;
    outline = Author::RunLiveCommand(request, registry, config, config.m_ProjectFolder,
                                     config.m_ActiveScenePath, ignored);
    if (glz::write_json(outline, outlineJson)) {
        outlineJson = "{}";
    }
    if (outlineJson.size() > kOutlineLimit) {
        outlineJson.resize(kOutlineLimit);
    }
    prompt = "You edit the scene that is already open in this editor. Call the tools. Do not write "
             "scene JSON yourself. Successful edits are written to the open scene file. If the "
             "user asks to keep the changes, leave that saved scene as it is.\n"
             "A button caption is set_ui_props props.label, or a top-level label. The prop named "
             "text is a color [r, g, b, a], not the caption. Renaming the entity does not change "
             "the caption. Read uiProps on the outline before changing a widget.\n"
             "Project folder: " +
             config.m_ProjectFolder + "\nScene: " + config.m_ActiveScenePath + "\nOutline:\n" +
             outlineJson;
    if (!m_Extra.empty()) {
        prompt += "\nExtra instructions:\n" + m_Extra;
    }
    messages += SystemMessageJson(prompt);
    first = false;
    for (const std::string& item : m_History) {
        if (item.empty()) {
            continue;
        }
        if (!first) {
            messages += ",";
        }
        messages += item;
        first = false;
    }
    messages += "]";
    (void)first;
    return messages;
}

std::string ChatPanel::RunTool(const ChatToolCall& call, Engine::Registry& registry,
                               EditorConfig& config)
{
    Author::AuthorRequest request;
    std::string args = call.arguments.empty() ? "{}" : call.arguments;
    glz::generic result;
    std::string json;

    if (glz::read_json(request, args)) {
        return "{\"ok\":false,\"error\":\"tool arguments were not JSON\"}";
    }
    request.apiVersion = Author::kApiVersion;
    request.command = call.name;
    if (request.projectFolder.empty()) {
        request.projectFolder = config.m_ProjectFolder;
    }
    if (request.scenePath.empty()) {
        request.scenePath = config.m_ActiveScenePath;
    }
    if (request.engineRoot.empty()) {
        request.engineRoot = RTYPE_SOURCE_DIR;
    }
    result = Author::RunLiveCommand(request, registry, config, config.m_ProjectFolder,
                                    config.m_ActiveScenePath, m_Files);
    if (glz::write_json(result, json)) {
        return "{\"ok\":false,\"error\":\"could not encode the tool result\"}";
    }
    return json;
}

void ChatPanel::ApplyCredentials()
{
    m_ApiKey = CompactKey(m_DraftKey);
    m_Model = Trim(m_DraftModel);
    m_Url = Trim(m_DraftUrl);
    m_Extra = m_DraftExtra;
}

void ChatPanel::StartRequest(const std::string& messagesJson)
{
    ChatRequest request;

    ApplyCredentials();
    request.apiKey = m_ApiKey;
    request.url = m_Url;
    request.model = m_Model;
    request.messagesJson = messagesJson;
    request.toolsJson = m_Tools;
    m_Waiting = true;
    m_PendingReady = false;
    if (m_Thread.joinable()) {
        m_Thread.join();
    }
    m_Thread = std::thread([this, request]() {
        ChatCompletion completion = CompleteChat(request);
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_Pending = std::move(completion);
        m_PendingReady = true;
    });
}

void ChatPanel::FinishTurn(Engine::Registry& registry, EditorConfig& config, const std::string& text)
{
    if (!text.empty()) {
        m_Lines.push_back({"assistant", text});
        m_History.push_back(AssistantMessageJson(ChatCompletion{text, {}, {}}));
    }
    m_Waiting = false;
    m_Rounds = 0;
    if (SceneChanged(registry, config)) {
        if (SaveScene(registry, config)) {
            m_Lines.push_back(
                {"info", "Saved the scene. Undo puts it back. Keep leaves this version."});
        }
        m_Review = true;
    } else {
        ClearSnapshot();
    }
}

void ChatPanel::ApplyCompletion(ChatCompletion completion, Engine::Registry& registry,
                                EditorConfig& config)
{
    std::string assistant;

    m_Waiting = false;
    if (!completion.error.empty() && completion.content.empty() && completion.toolCalls.empty()) {
        m_Lines.push_back({"error", completion.error});
        FinishTurn(registry, config, "");
        return;
    }
    if (!completion.toolCalls.empty()) {
        if (m_Rounds >= kMaxToolRounds) {
            m_Lines.push_back({"error", "Stopped after 8 tool rounds."});
            FinishTurn(registry, config, completion.content);
            return;
        }
        ++m_Rounds;
        assistant = AssistantMessageJson(completion);
        m_History.push_back(assistant);
        if (!completion.content.empty()) {
            m_Lines.push_back({"assistant", completion.content});
        }
        for (const ChatToolCall& call : completion.toolCalls) {
            std::string result = RunTool(call, registry, config);
            m_History.push_back(ToolMessageJson(call.id, result));
            m_Lines.push_back({"tool", call.name + " " + ShortText(result)});
        }
        StartRequest(BuildMessages(registry, config));
        return;
    }
    FinishTurn(registry, config, completion.content.empty() ? completion.error : completion.content);
}

void ChatPanel::Poll(Engine::Registry& registry, EditorConfig& config)
{
    ChatCompletion completion;
    bool ready = false;

    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        if (m_PendingReady) {
            completion = std::move(m_Pending);
            m_PendingReady = false;
            ready = true;
        }
    }
    if (!ready) {
        return;
    }
    if (m_Thread.joinable()) {
        m_Thread.join();
    }
    ApplyCompletion(std::move(completion), registry, config);
}

void ChatPanel::Send(Engine::Registry& registry, EditorConfig& config)
{
    std::string text = m_Input;
    bool starting = false;

    if (m_Waiting) {
        return;
    }
    if (text.empty()) {
        return;
    }
    if (m_Review && IsKeepRequest(text)) {
        m_Input.clear();
        m_Lines.push_back({"user", text});
        if (AcceptEdit(registry, config)) {
            m_Lines.push_back({"info", "Kept the edit and saved the scene."});
        }
        return;
    }
    starting = !m_HasSnapshot;
    if (m_ApiKey.empty()) {
        m_Lines.push_back({"error", "API key is empty. Open the gear and paste a key."});
        return;
    }
    if (config.m_ActiveScenePath.empty() || config.m_ProjectFolder.empty()) {
        m_Lines.push_back({"error", "Open a project scene first."});
        return;
    }
    m_Input.clear();
    m_Lines.push_back({"user", text});
    m_History.push_back(UserMessageJson(text));
    if (starting) {
        Snapshot(registry, config);
    }
    m_Rounds = 0;
    StartRequest(BuildMessages(registry, config));
}

void ChatPanel::OnImGuiRender(Engine::Registry& registry, EditorConfig& config)
{
    Poll(registry, config);
    if (!ImGui::Begin(m_Name.c_str())) {
        ImGui::End();
        return;
    }
    if (ImGui::Button(ICON_FA_GEAR)) {
        ImGui::OpenPopup("ChatSettings");
    }
    if (ImGui::BeginPopup("ChatSettings")) {
        bool edited = false;

        if (!m_SettingsEditing) {
            m_DraftKey = m_ApiKey;
            m_DraftModel = m_Model;
            m_DraftUrl = m_Url;
            m_DraftExtra = m_Extra;
            m_DraftKey.reserve(512);
            m_SettingsEditing = true;
        }
        edited = ImGui::InputText("API key", &m_DraftKey, ImGuiInputTextFlags_Password);
        edited = ImGui::InputText("Model", &m_DraftModel) || edited;
        edited = ImGui::InputText("Endpoint", &m_DraftUrl) || edited;
        edited = ImGui::InputTextMultiline("Instructions", &m_DraftExtra, ImVec2(0.0f, 80.0f)) ||
                 edited;
        if (edited) {
            m_SettingsSaved = false;
        }
        if (ImGui::Button("Save")) {
            ApplyCredentials();
            m_SettingsSaved = true;
        }
        ImGui::SameLine();
        if (!m_SettingsSaved) {
            ImGui::TextDisabled("Not saved yet.");
        } else if (m_ApiKey.empty()) {
            ImGui::TextDisabled("Saved. No key.");
        } else {
            ImGui::TextDisabled("Saved for this session (%d characters).",
                                static_cast<int>(m_ApiKey.size()));
        }
        ImGui::EndPopup();
    } else if (m_SettingsEditing) {
        m_SettingsEditing = false;
    }
    ImGui::SameLine();
    if (m_ApiKey.empty()) {
        ImGui::TextDisabled("No key");
    } else {
        ImGui::TextUnformatted(m_Model.c_str());
    }
    if (ImGui::BeginChild("ChatLog", ImVec2(0.0f, -ImGui::GetFrameHeightWithSpacing() * 4.0f), true)) {
        for (const Line& line : m_Lines) {
            if (line.role == "user") {
                ImGui::TextWrapped("You: %s", line.text.c_str());
            } else if (line.role == "assistant") {
                ImGui::TextWrapped("Chat: %s", line.text.c_str());
            } else if (line.role == "error") {
                ImGui::TextWrapped("Error: %s", line.text.c_str());
            } else if (line.role == "tool") {
                ImGui::TextDisabled("%s", line.text.c_str());
            } else {
                ImGui::TextWrapped("%s", line.text.c_str());
            }
        }
        if (m_Waiting) {
            ImGui::TextDisabled("Waiting...");
        }
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
            ImGui::SetScrollHereY(1.0f);
        }
    }
    ImGui::EndChild();
    if (m_Review) {
        if (ImGui::Button("Undo")) {
            Undo(registry, config);
        }
        ImGui::SameLine();
        if (ImGui::Button("Keep")) {
            if (AcceptEdit(registry, config)) {
                m_Lines.push_back({"info", "Kept the edit and saved the scene."});
            }
        }
    }
    ImGui::BeginDisabled(m_Waiting);
    ImGui::InputTextMultiline("##ChatInput", &m_Input, ImVec2(-1.0f, ImGui::GetFrameHeight() * 2.0f));
    if (ImGui::Button("Send") && !m_Waiting) {
        Send(registry, config);
    }
    ImGui::EndDisabled();
    ImGui::End();
}

} // namespace Editor
