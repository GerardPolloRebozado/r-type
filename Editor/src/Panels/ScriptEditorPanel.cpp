#include "Panels/ScriptEditorPanel.hpp"

#include "EditorLayer.hpp"
#include "ScriptEditorService.hpp"
#include "Scripting/LuaLanguage.hpp"
#include "Scripting/LuaSyntax.hpp"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <extras/IconsFontAwesome6.h>
#include <fstream>
#include <imgui.h>
#include <iterator>
#include <misc/cpp/imgui_stdlib.h>
#include <system_error>
#include <utility>

namespace fs = std::filesystem;

namespace Editor {
namespace {

constexpr int kTabSize = 4;
constexpr double kSyntaxCheckDelay = 0.35;
constexpr double kExternalCheckInterval = 1.0;

std::string ToLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

bool IsLuaFile(const fs::path& path) {
    return ToLower(path.extension().string()) == ".lua";
}

fs::path NormalizePath(const fs::path& path, const fs::path& base) {
    const fs::path absolute = path.is_absolute() ? path : base / path;
    std::error_code ec;
    const fs::path canonical = fs::weakly_canonical(absolute, ec);
    return ec ? absolute.lexically_normal() : canonical;
}

bool IsInsideProject(const fs::path& path, const fs::path& project) {
    std::error_code ec;
    const fs::path relative = fs::relative(path, project, ec);
    if (ec || relative.empty()) {
        return false;
    }
    for (const fs::path& part : relative) {
        if (part == "..") {
            return false;
        }
    }
    return true;
}

bool ReadFile(const fs::path& path, std::string& text, bool& utf8Bom, std::string& lineEnding,
              std::string& error) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        error = "Could not open " + path.string();
        return false;
    }

    std::string raw((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    if (input.bad()) {
        error = "Could not read " + path.string();
        return false;
    }

    utf8Bom = raw.size() >= 3 && static_cast<unsigned char>(raw[0]) == 0xEF &&
              static_cast<unsigned char>(raw[1]) == 0xBB &&
              static_cast<unsigned char>(raw[2]) == 0xBF;
    text = utf8Bom ? raw.substr(3) : std::move(raw);
    lineEnding = text.find("\r\n") != std::string::npos ? "\r\n" : "\n";
    return true;
}

bool WriteFile(const fs::path& path, const std::string& text, bool utf8Bom,
               const std::string& lineEnding, std::string& error) {
    std::string contents;
    contents.reserve(text.size() + (text.size() / 32) + 3);
    if (lineEnding == "\r\n") {
        for (const char c : text) {
            if (c == '\n') {
                contents += '\r';
            }
            contents += c;
        }
    } else {
        contents = text;
    }
    if (utf8Bom) {
        contents.insert(0, "\xEF\xBB\xBF");
    }

    fs::path temporary = path;
    temporary += ".tmp";

    std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
    if (!output) {
        error = "Could not create " + temporary.string();
        return false;
    }
    output.write(contents.data(), static_cast<std::streamsize>(contents.size()));
    output.close();
    if (!output) {
        std::error_code removeError;
        fs::remove(temporary, removeError);
        error = "Could not write " + temporary.string();
        return false;
    }

    std::error_code renameError;
    fs::rename(temporary, path, renameError);
    if (renameError) {
        // Some platforms refuse to replace an existing file with rename().
        std::error_code removeError;
        fs::remove(path, removeError);
        renameError.clear();
        fs::rename(temporary, path, renameError);
    }
    if (renameError) {
        std::error_code removeError;
        fs::remove(temporary, removeError);
        error = "Could not replace " + path.string();
        return false;
    }
    return true;
}

int Utf8SequenceLength(unsigned char c) {
    if ((c & 0x80U) == 0U) {
        return 1;
    }
    if ((c & 0xE0U) == 0xC0U) {
        return 2;
    }
    if ((c & 0xF0U) == 0xE0U) {
        return 3;
    }
    if ((c & 0xF8U) == 0xF0U) {
        return 4;
    }
    return 1;
}

std::string EditorTextForDisk(const TextEditor& editor) {
    std::string text = editor.GetText();
    if (!text.empty() && text.back() == '\n') {
        text.pop_back();
    }
    return text;
}

int ColumnForByteIndex(const std::string& line, std::size_t byteIndex) {
    int column = 0;
    std::size_t index = 0;
    while (index < byteIndex && index < line.size()) {
        if (line[index] == '\t') {
            column += kTabSize - (column % kTabSize);
            ++index;
            continue;
        }
        const int length = Utf8SequenceLength(static_cast<unsigned char>(line[index]));
        index += std::min<std::size_t>(static_cast<std::size_t>(length), line.size() - index);
        ++column;
    }
    return column;
}

bool IsCommandDown(const ImGuiIO& io) {
    return io.KeyCtrl || (io.ConfigMacOSXBehaviors && io.KeySuper);
}

} // namespace

ScriptEditorPanel::ScriptEditorPanel() = default;

ScriptEditorPanel::~ScriptEditorPanel() = default;

const std::string& ScriptEditorPanel::GetName() const {
    return m_Name;
}

void ScriptEditorPanel::Focus() {
    m_FocusRequested = true;
}

bool ScriptEditorPanel::HasDirtyDocuments() const {
    for (const auto& document : m_Documents) {
        if (document->dirty) {
            return true;
        }
    }
    return false;
}

bool ScriptEditorPanel::SaveAll() {
    bool success = true;
    for (auto& document : m_Documents) {
        if (document->dirty && !SaveDocument(*document)) {
            success = false;
        }
    }
    return success;
}

void ScriptEditorPanel::OnImGuiRender(Engine::Registry& registry, EditorConfig& config) {
    ConsumeOpenRequests(registry, config);

    if (m_FocusRequested) {
        ImGui::SetWindowFocus(m_Name.c_str());
    }

    if (!ImGui::Begin(m_Name.c_str())) {
        ImGui::End();
        return;
    }

    DrawToolbar();
    DrawTabs();
    ProcessCloseRequest();

    const bool hasActive =
        m_ActiveDocument >= 0 && m_ActiveDocument < static_cast<int>(m_Documents.size());
    DrawStatusBar(hasActive ? m_Documents[static_cast<std::size_t>(m_ActiveDocument)].get()
                            : nullptr);

    m_FocusRequested = false;

    DrawClosePopup();
    DrawGoToLinePopup();

    ImGui::End();
}

void ScriptEditorPanel::ConsumeOpenRequests(Engine::Registry& registry, EditorConfig& config) {
    if (!registry.HasResource<ScriptEditorService>()) {
        return;
    }

    ScriptEditorService& service = registry.GetResource<ScriptEditorService>();
    for (const ScriptOpenRequest& request : service.ConsumeOpenRequests()) {
        OpenFile(request.path, config);
    }
    if (service.ConsumeFocusRequest()) {
        m_FocusRequested = true;
    }
    if (auto fontSizeReq = service.ConsumeFontSizeChangeRequest()) {
        for (auto& document : m_Documents) {
            document->fontSize = fontSizeReq->fontSize;
        }
        config.m_ScriptFontSize = fontSizeReq->fontSize;
    }
}

void ScriptEditorPanel::OpenFile(const fs::path& requested, EditorConfig& config) {
    if (config.m_ProjectFolder.empty()) {
        SetStatus("Cannot open scripts: no project folder is configured.", true);
        return;
    }

    const fs::path project = NormalizePath(config.m_ProjectFolder, fs::current_path());
    if (!IsLuaFile(requested)) {
        SetStatus("Script Editor can only open .lua files.", true);
        return;
    }

    const fs::path absolute = NormalizePath(requested, project);
    if (!IsInsideProject(absolute, project)) {
        SetStatus("Script Editor can only open files inside the current project.", true);
        return;
    }

    std::error_code fileError;
    if (!fs::is_regular_file(absolute, fileError)) {
        SetStatus("Could not find " + absolute.string(), true);
        return;
    }

    for (std::size_t i = 0; i < m_Documents.size(); ++i) {
        if (m_Documents[i]->path == absolute) {
            m_ActiveDocument = static_cast<int>(i);
            m_FocusRequested = true;
            return;
        }
    }

    std::error_code relativeError;
    const fs::path relative = fs::relative(absolute, project, relativeError);
    if (relativeError) {
        SetStatus("Could not resolve the script path.", true);
        return;
    }

    auto document = std::make_unique<Document>();
    document->path = absolute;
    document->displayPath = relative.generic_string();
    document->editor = std::make_unique<TextEditor>();
    document->editor->SetLanguageDefinition(MakeLuaLanguageDefinition());
    document->editor->SetPalette(MakeLuaEditorPalette());
    document->editor->SetTabSize(kTabSize);
    document->editor->SetTextStart(52.0f);
    document->editor->SetShowWhitespaces(m_ShowWhitespace);
    document->fontSize = config.m_ScriptFontSize;

    if (!LoadDocument(*document)) {
        return;
    }

    m_Documents.push_back(std::move(document));
    m_ActiveDocument = static_cast<int>(m_Documents.size()) - 1;
    m_FocusRequested = true;
}

bool ScriptEditorPanel::LoadDocument(Document& document) {
    std::string text;
    std::string error;
    bool bom = false;
    std::string lineEnding = "\n";
    if (!ReadFile(document.path, text, bom, lineEnding, error)) {
        SetStatus(error, true);
        document.missing = true;
        return false;
    }

    document.editor->SetText(text);
    document.editor->SetSelection(TextEditor::Coordinates(0, 0), TextEditor::Coordinates(0, 0));
    document.editor->SetCursorPosition(TextEditor::Coordinates(0, 0));
    document.lineEnding = lineEnding;
    document.utf8Bom = bom;
    document.dirty = false;
    document.missing = false;
    document.externalChanged = false;
    document.syntaxCheckPending = false;
    document.lastEditTime = 0.0;
    document.lastExternalCheck = ImGui::GetTime();
    RefreshFileMetadata(document);

    CheckSyntax(document);
    return true;
}

bool ScriptEditorPanel::SaveDocument(Document& document) {
    if (!document.editor) {
        return false;
    }

    const std::string text = EditorTextForDisk(*document.editor);
    std::string error;
    if (!WriteFile(document.path, text, document.utf8Bom, document.lineEnding, error)) {
        SetStatus(error, true, 6.0);
        return false;
    }

    document.dirty = false;
    document.missing = false;
    document.externalChanged = false;
    document.syntaxCheckPending = false;
    RefreshFileMetadata(document);
    CheckSyntax(document);
    SetStatus("Saved " + document.displayPath);
    return true;
}

bool ScriptEditorPanel::SaveDocumentAt(int index) {
    if (index < 0 || index >= static_cast<int>(m_Documents.size())) {
        return false;
    }
    return SaveDocument(*m_Documents[static_cast<std::size_t>(index)]);
}

void ScriptEditorPanel::ReloadDocument(Document& document) {
    if (!LoadDocument(document)) {
        return;
    }
    SetStatus("Reloaded " + document.displayPath);
}

void ScriptEditorPanel::CloseDocument(int index) {
    if (index < 0 || index >= static_cast<int>(m_Documents.size())) {
        return;
    }

    m_Documents.erase(m_Documents.begin() + index);
    if (m_Documents.empty()) {
        m_ActiveDocument = -1;
        m_FindOpen = false;
    } else if (m_ActiveDocument > index) {
        --m_ActiveDocument;
    } else if (m_ActiveDocument == index) {
        m_ActiveDocument = std::min(index, static_cast<int>(m_Documents.size()) - 1);
    }
}

void ScriptEditorPanel::RequestClose(int index) {
    if (index >= 0 && index < static_cast<int>(m_Documents.size())) {
        m_TabCloseRequest = index;
    }
}

void ScriptEditorPanel::ProcessCloseRequest() {
    if (m_TabCloseRequest < 0) {
        return;
    }

    const int index = m_TabCloseRequest;
    m_TabCloseRequest = -1;
    if (index < 0 || index >= static_cast<int>(m_Documents.size())) {
        return;
    }

    if (m_Documents[static_cast<std::size_t>(index)]->dirty) {
        m_PendingClose = index;
        m_ClosePopupRequested = true;
    } else {
        CloseDocument(index);
    }
}

void ScriptEditorPanel::DrawToolbar() {
    const bool hasDocuments = !m_Documents.empty();
    const bool hasActive =
        m_ActiveDocument >= 0 && m_ActiveDocument < static_cast<int>(m_Documents.size());

    // Left side: status message
    if (!m_StatusMessage.empty() && ImGui::GetTime() < m_StatusUntil) {
        const ImVec4 color =
            m_StatusIsError ? ImVec4(1.0f, 0.38f, 0.38f, 1.0f) : ImVec4(0.62f, 0.82f, 0.62f, 1.0f);
        ImGui::TextColored(color, "%s", m_StatusMessage.c_str());
    }

    // Right side: build button group and position from right edge
    const float buttonSpacing = ImGui::GetStyle().ItemSpacing.x;
    const float padding = ImGui::GetStyle().WindowPadding.x;

    // Calculate total width of right-aligned controls
    float rightControlsWidth = 0.0f;
    // Save button
    rightControlsWidth += ImGui::CalcTextSize(ICON_FA_FLOPPY_DISK " Save").x +
                          ImGui::GetStyle().FramePadding.x * 2.0f;
    rightControlsWidth += buttonSpacing;
    // Save All button
    rightControlsWidth +=
        ImGui::CalcTextSize("Save All").x + ImGui::GetStyle().FramePadding.x * 2.0f;
    rightControlsWidth += buttonSpacing;
    // Find button
    rightControlsWidth += ImGui::CalcTextSize(ICON_FA_MAGNIFYING_GLASS " Find").x +
                          ImGui::GetStyle().FramePadding.x * 2.0f;
    rightControlsWidth += buttonSpacing;
    // Replace button
    rightControlsWidth +=
        ImGui::CalcTextSize("Replace").x + ImGui::GetStyle().FramePadding.x * 2.0f;
    rightControlsWidth += buttonSpacing;
    // Go to Line button
    rightControlsWidth +=
        ImGui::CalcTextSize("Go to Line").x + ImGui::GetStyle().FramePadding.x * 2.0f;
    rightControlsWidth += buttonSpacing;
    // Whitespace checkbox
    rightControlsWidth += ImGui::CalcTextSize("Whitespace").x +
                          ImGui::GetStyle().FramePadding.x * 2.0f + ImGui::GetFrameHeight();
    rightControlsWidth += padding;

    // Position cursor at right edge minus controls width
    float cursorX = ImGui::GetWindowWidth() - rightControlsWidth;
    if (cursorX < ImGui::GetCursorPosX()) {
        cursorX = ImGui::GetCursorPosX();
    }
    ImGui::SetCursorPosX(cursorX);

    ImGui::BeginDisabled(!hasActive);
    if (ImGui::Button(ICON_FA_FLOPPY_DISK " Save")) {
        SaveDocumentAt(m_ActiveDocument);
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    ImGui::BeginDisabled(!hasDocuments);
    if (ImGui::Button("Save All")) {
        SaveAll();
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    ImGui::BeginDisabled(!hasActive);
    if (ImGui::Button(ICON_FA_MAGNIFYING_GLASS " Find")) {
        if (hasActive &&
            m_Documents[static_cast<std::size_t>(m_ActiveDocument)]->editor->HasSelection()) {
            const std::string selected =
                m_Documents[static_cast<std::size_t>(m_ActiveDocument)]->editor->GetSelectedText();
            if (!selected.empty() && selected.find('\n') == std::string::npos) {
                m_FindText = selected;
            }
        }
        m_FindOpen = true;
        m_ReplaceOpen = false;
        m_FindFocusRequested = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Replace")) {
        m_FindOpen = true;
        m_ReplaceOpen = true;
        m_FindFocusRequested = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Go to Line")) {
        if (hasActive) {
            m_GoToLine = m_Documents[static_cast<std::size_t>(m_ActiveDocument)]
                             ->editor->GetCursorPosition()
                             .mLine +
                         1;
        }
        m_GoToLineOpen = true;
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    ImGui::Checkbox("Whitespace", &m_ShowWhitespace);
}

void ScriptEditorPanel::DrawTabs() {
    if (m_Documents.empty()) {
        ImGui::TextDisabled("Open a .lua file from Assets or the Inspector to start editing.");
        return;
    }

    if (!ImGui::BeginTabBar("##ScriptEditorTabs",
                            ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_AutoSelectNewTabs)) {
        return;
    }

    for (int i = 0; i < static_cast<int>(m_Documents.size()); ++i) {
        Document& document = *m_Documents[static_cast<std::size_t>(i)];
        std::string title = document.path.filename().string();
        if (document.dirty) {
            title += " *";
        }
        title += "###" + document.displayPath;

        bool open = true;
        ImGuiTabItemFlags flags = ImGuiTabItemFlags_None;
        if (m_FocusRequested && i == m_ActiveDocument) {
            flags |= ImGuiTabItemFlags_SetSelected;
        }

        if (ImGui::BeginTabItem(title.c_str(), &open, flags)) {
            if (m_ActiveDocument != i) {
                m_ActiveDocument = i;
            }
            DrawDocument(document);
            ImGui::EndTabItem();
        }

        if (!open) {
            RequestClose(i);
        }
    }

    ImGui::EndTabBar();
}

void ScriptEditorPanel::DrawDocument(Document& document) {
    if (document.missing) {
        ImGui::TextColored(ImVec4(1.0f, 0.42f, 0.42f, 1.0f), "This file is missing on disk.");
        ImGui::SameLine();
        if (ImGui::Button("Save")) {
            SaveDocument(document);
        }
    }

    if (document.externalChanged) {
        ImGui::TextColored(ImVec4(1.0f, 0.78f, 0.30f, 1.0f), "This file changed on disk.");
        ImGui::SameLine();
        if (ImGui::Button("Reload")) {
            ReloadDocument(document);
        }
        ImGui::SameLine();
        if (ImGui::Button("Keep Editor Version")) {
            document.externalChanged = false;
            RefreshFileMetadata(document);
        }
    }

    if (m_FindOpen) {
        DrawFindBar(document);
    }

    HandleShortcuts(document);

    ImVec2 available = ImGui::GetContentRegionAvail();
    // The status bar is drawn below the tab bar, so reserve one line here.
    available.y = std::max(40.0f, available.y - ImGui::GetFrameHeightWithSpacing());

    const std::string editorId = "##ScriptText_" + document.displayPath;
    ImFont* codeFont = g_CodeFont != nullptr ? g_CodeFont : ImGui::GetFont();
    ImGui::PushFont(codeFont, document.fontSize);
    document.editor->SetShowWhitespaces(m_ShowWhitespace);
    document.editor->Render(editorId.c_str(), available, true);
    ImGui::PopFont();

    UpdateDocumentState(document);
}

void ScriptEditorPanel::DrawFindBar(Document& document) {
    ImGui::PushID("ScriptEditorFind");
    if (m_FindFocusRequested) {
        ImGui::SetKeyboardFocusHere();
        m_FindFocusRequested = false;
    }

    ImGui::SetNextItemWidth(220.0f);
    const bool searchEnter = ImGui::InputTextWithHint("##Find", "Find", &m_FindText,
                                                      ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_ARROW_LEFT)) {
        FindNext(document, true);
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_ARROW_RIGHT)) {
        FindNext(document, false);
    }
    ImGui::SameLine();
    ImGui::Checkbox("Case", &m_FindCaseSensitive);
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_XMARK "##CloseFind")) {
        m_FindOpen = false;
    }

    if (m_ReplaceOpen) {
        ImGui::SetNextItemWidth(220.0f);
        const bool replaceEnter = ImGui::InputTextWithHint("##Replace", "Replace", &m_ReplaceText,
                                                           ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::SameLine();
        if (ImGui::Button("Replace")) {
            ReplaceCurrent(document);
        }
        ImGui::SameLine();
        if (ImGui::Button("Replace All")) {
            ReplaceAll(document);
        }
        if (replaceEnter) {
            ReplaceCurrent(document);
        }
    }

    if (searchEnter) {
        FindNext(document, ImGui::GetIO().KeyShift);
    }
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
        ImGui::IsKeyPressed(ImGuiKey_Escape, false)) {
        m_FindOpen = false;
    }

    ImGui::PopID();
}

void ScriptEditorPanel::DrawStatusBar(const Document* document) {
    if (document == nullptr || !document->editor) {
        return;
    }

    const TextEditor::Coordinates cursor = document->editor->GetCursorPosition();
    ImGui::Separator();
    ImGui::TextUnformatted(document->displayPath.c_str());
    ImGui::SameLine();
    ImGui::TextDisabled("Ln %d, Col %d", cursor.mLine + 1, cursor.mColumn + 1);
    ImGui::SameLine();
    ImGui::TextDisabled("%s", document->lineEnding == "\r\n" ? "CRLF" : "LF");
    if (document->utf8Bom) {
        ImGui::SameLine();
        ImGui::TextDisabled("UTF-8 BOM");
    }
    if (document->dirty) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.78f, 0.30f, 1.0f), "Modified");
    }
    if (document->errorCount > 0) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.42f, 0.42f, 1.0f), "%zu syntax error(s)",
                           document->errorCount);
    }
}

void ScriptEditorPanel::DrawClosePopup() {
    if (m_PendingClose < 0) {
        return;
    }
    if (m_PendingClose >= static_cast<int>(m_Documents.size())) {
        m_PendingClose = -1;
        return;
    }

    if (m_ClosePopupRequested) {
        ImGui::OpenPopup("Close Script");
        m_ClosePopupRequested = false;
    }

    if (!ImGui::BeginPopupModal("Close Script", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        if (!m_ClosePopupRequested && !ImGui::IsPopupOpen("Close Script")) {
            m_PendingClose = -1;
        }
        return;
    }

    Document& document = *m_Documents[static_cast<std::size_t>(m_PendingClose)];
    ImGui::Text("Save changes to \"%s\" before closing?", document.displayPath.c_str());
    ImGui::Spacing();

    if (ImGui::Button("Save")) {
        if (SaveDocument(document)) {
            const int index = m_PendingClose;
            m_PendingClose = -1;
            ImGui::CloseCurrentPopup();
            CloseDocument(index);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Discard")) {
        const int index = m_PendingClose;
        m_PendingClose = -1;
        ImGui::CloseCurrentPopup();
        CloseDocument(index);
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) {
        m_PendingClose = -1;
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void ScriptEditorPanel::DrawGoToLinePopup() {
    if (m_GoToLineOpen) {
        m_GoToLineOpen = false;
        m_GoToLineFocusRequested = true;
        ImGui::OpenPopup("Go to Line");
    }

    if (!ImGui::BeginPopupModal("Go to Line", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        return;
    }

    if (m_GoToLineFocusRequested) {
        ImGui::SetKeyboardFocusHere();
        m_GoToLineFocusRequested = false;
    }

    bool go = ImGui::InputInt("Line", &m_GoToLine, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue);
    if (ImGui::Button("Go")) {
        go = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) {
        ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
        return;
    }

    if (go && m_ActiveDocument >= 0 && m_ActiveDocument < static_cast<int>(m_Documents.size())) {
        TextEditor& editor = *m_Documents[static_cast<std::size_t>(m_ActiveDocument)]->editor;
        const int line = std::clamp(m_GoToLine, 1, std::max(1, editor.GetTotalLines()));
        const TextEditor::Coordinates target(line - 1, 0);
        editor.SetSelection(target, target);
        editor.SetCursorPosition(target);
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void ScriptEditorPanel::HandleShortcuts(Document& document) {
    if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) {
        return;
    }

    const ImGuiIO& io = ImGui::GetIO();
    const bool cmd = IsCommandDown(io);

    // Zoom shortcuts (work regardless of cmd modifier for mouse wheel)
    if (io.MouseWheel != 0.0f && cmd) {
        document.fontSize = std::clamp(document.fontSize + io.MouseWheel * 1.0f, 8.0f, 32.0f);
    }
    if (cmd) {
        if (ImGui::IsKeyPressed(ImGuiKey_Equal, false) ||
            ImGui::IsKeyPressed(ImGuiKey_KeypadAdd, false)) {
            document.fontSize = std::clamp(document.fontSize + 1.0f, 8.0f, 32.0f);
        } else if (ImGui::IsKeyPressed(ImGuiKey_Minus, false) ||
                   ImGui::IsKeyPressed(ImGuiKey_KeypadSubtract, false)) {
            document.fontSize = std::clamp(document.fontSize - 1.0f, 8.0f, 32.0f);
        } else if (ImGui::IsKeyPressed(ImGuiKey_0, false)) {
            document.fontSize = 15.0f;
        } else if (ImGui::IsKeyPressed(ImGuiKey_D, false)) {
            // Duplicate line (Ctrl+D)
            DuplicateLine(document);
        } else if (ImGui::IsKeyPressed(ImGuiKey_N, false)) {
            // New file (Ctrl+N)
            NewFile();
        }
    }

    if (!cmd) {
        if (!m_FindText.empty() && ImGui::IsKeyPressed(ImGuiKey_F3, false)) {
            FindNext(document, io.KeyShift);
        }
        return;
    }

    // Comment toggle (Ctrl+/)
    if (ImGui::IsKeyPressed(ImGuiKey_Slash, false)) {
        ToggleLineComments(document);
    }

    if (ImGui::IsKeyPressed(ImGuiKey_S, false)) {
        if (io.KeyShift) {
            SaveAll();
        } else {
            SaveDocument(document);
        }
    } else if (ImGui::IsKeyPressed(ImGuiKey_W, false)) {
        for (int i = 0; i < static_cast<int>(m_Documents.size()); ++i) {
            if (m_Documents[static_cast<std::size_t>(i)].get() == &document) {
                RequestClose(i);
                break;
            }
        }
    } else if (ImGui::IsKeyPressed(ImGuiKey_F, false)) {
        if (document.editor->HasSelection()) {
            const std::string selected = document.editor->GetSelectedText();
            if (!selected.empty() && selected.find('\n') == std::string::npos) {
                m_FindText = selected;
            }
        }
        m_FindOpen = true;
        m_ReplaceOpen = false;
        m_FindFocusRequested = true;
    } else if (ImGui::IsKeyPressed(ImGuiKey_H, false)) {
        m_FindOpen = true;
        m_ReplaceOpen = true;
        m_FindFocusRequested = true;
    } else if (ImGui::IsKeyPressed(ImGuiKey_G, false)) {
        m_GoToLine = document.editor->GetCursorPosition().mLine + 1;
        m_GoToLineOpen = true;
    } else if (ImGui::IsKeyPressed(ImGuiKey_F3, false)) {
        FindNext(document, io.KeyShift);
    }
}

void ScriptEditorPanel::DuplicateLine(Document& document) {
    if (!document.editor) {
        return;
    }

    const int line = document.editor->GetCursorPosition().mLine;
    const std::string lineText = GetTextLine(document, line);

    // Move cursor to end of line and insert newline + duplicated content
    document.editor->SetCursorPosition(
        TextEditor::Coordinates(line, static_cast<int>(lineText.size())));
    document.editor->InsertText("\n" + lineText);
    document.dirty = true;
    document.syntaxCheckPending = true;
    document.lastEditTime = ImGui::GetTime();
}

void ScriptEditorPanel::NewFile() {
    auto document = std::make_unique<Document>();
    document->displayPath = "untitled.lua";
    document->path = ""; // No path until saved
    document->editor = std::make_unique<TextEditor>();
    document->editor->SetLanguageDefinition(MakeLuaLanguageDefinition());
    document->editor->SetPalette(MakeLuaEditorPalette());
    document->editor->SetTabSize(kTabSize);
    document->editor->SetTextStart(52.0f);
    document->editor->SetShowWhitespaces(m_ShowWhitespace);
    document->fontSize = 15.0f;
    document->lineEnding = "\n";
    document->utf8Bom = false;
    document->dirty = true;
    document->missing = false;
    document->externalChanged = false;
    document->syntaxCheckPending = false;
    document->lastEditTime = 0.0;
    document->lastExternalCheck = ImGui::GetTime();

    m_Documents.push_back(std::move(document));
    m_ActiveDocument = static_cast<int>(m_Documents.size()) - 1;
    m_FocusRequested = true;
}

void ScriptEditorPanel::ToggleLineComments(Document& document) {
    if (!document.editor) {
        return;
    }

    const bool hasSelection = document.editor->HasSelection();

    int startLine, endLine;
    if (hasSelection) {
        const auto cursor = document.editor->GetCursorPosition();
        // we don't have direct access to selection start, so approximate
        // if there's a selection, use cursor line as end, and find start by scanning
        // for simplicity, just use the current cursor line if we can't determine range
        startLine = document.editor->GetCursorPosition().mLine;
        endLine = startLine;
    } else {
        startLine = document.editor->GetCursorPosition().mLine;
        endLine = startLine;
    }

    bool allCommented = true;
    for (int line = startLine; line <= endLine; ++line) {
        const std::string text = GetTextLine(document, line);
        std::string trimmed = text;
        trimmed.erase(0, trimmed.find_first_not_of(" \t"));
        if (!trimmed.starts_with("--")) {
            allCommented = false;
            break;
        }
    }

    std::string newText;
    for (int line = startLine; line <= endLine; ++line) {
        std::string text = GetTextLine(document, line);
        if (allCommented) {
            size_t pos = text.find("--");
            if (pos != std::string::npos) {
                text.erase(pos, 2);
                if (pos < text.size() && text[pos] == ' ') {
                    text.erase(pos, 1);
                }
            }
        } else {
            size_t firstNonWs = text.find_first_not_of(" \t");
            if (firstNonWs != std::string::npos) {
                text.insert(firstNonWs, "-- ");
            } else {
                text = "-- " + text;
            }
        }
        newText += text;
        if (line < endLine) {
            newText += "\n";
        }
    }

    document.editor->SetSelection(
        TextEditor::Coordinates(startLine, 0),
        TextEditor::Coordinates(endLine, static_cast<int>(GetTextLine(document, endLine).size())));
    document.editor->ReplaceSelection(newText);
    document.dirty = true;
    document.syntaxCheckPending = true;
    document.lastEditTime = ImGui::GetTime();
}

void ScriptEditorPanel::UpdateDocumentState(Document& document) {
    if (document.editor->IsTextChanged()) {
        document.dirty = true;
        document.syntaxCheckPending = true;
        document.lastEditTime = ImGui::GetTime();
    }

    if (document.syntaxCheckPending &&
        ImGui::GetTime() - document.lastEditTime >= kSyntaxCheckDelay) {
        CheckSyntax(document);
    }

    PollExternalChange(document);
}

void ScriptEditorPanel::CheckSyntax(Document& document) {
    const std::vector<LuaDiagnostic> diagnostics =
        CheckLuaSyntax(EditorTextForDisk(*document.editor), document.displayPath);

    TextEditor::ErrorMarkers markers;
    for (const LuaDiagnostic& diagnostic : diagnostics) {
        std::string& message = markers[std::max(1, diagnostic.line)];
        if (!message.empty()) {
            message += "\n";
        }
        message += diagnostic.message;
    }

    document.editor->SetErrorMarkers(markers);
    document.errorCount = diagnostics.size();
    document.syntaxCheckPending = false;
}

void ScriptEditorPanel::PollExternalChange(Document& document) {
    const double now = ImGui::GetTime();
    if (now - document.lastExternalCheck < kExternalCheckInterval) {
        return;
    }
    document.lastExternalCheck = now;

    std::error_code timeError;
    const fs::file_time_type modified = fs::last_write_time(document.path, timeError);
    if (timeError) {
        document.missing = true;
        document.externalChanged = false;
        return;
    }

    std::error_code sizeError;
    const std::uintmax_t size = fs::file_size(document.path, sizeError);
    if (sizeError) {
        return;
    }

    const bool changed =
        document.missing || modified != document.modifiedTime || size != document.fileSize;
    if (!changed) {
        return;
    }

    if (document.dirty) {
        document.missing = false;
        document.externalChanged = true;
        return;
    }

    ReloadDocument(document);
}

void ScriptEditorPanel::SetStatus(std::string message, bool isError, double seconds) {
    m_StatusMessage = std::move(message);
    m_StatusIsError = isError;
    m_StatusUntil = ImGui::GetTime() + seconds;
}

void ScriptEditorPanel::RefreshFileMetadata(Document& document) {
    std::error_code timeError;
    const fs::file_time_type modified = fs::last_write_time(document.path, timeError);
    if (!timeError) {
        document.modifiedTime = modified;
    }

    std::error_code sizeError;
    const std::uintmax_t size = fs::file_size(document.path, sizeError);
    if (!sizeError) {
        document.fileSize = size;
    }
}

std::vector<ScriptEditorPanel::Match>
ScriptEditorPanel::FindMatches(const Document& document) const {
    std::vector<Match> matches;
    if (!document.editor || m_FindText.empty()) {
        return matches;
    }

    const std::string needle = m_FindCaseSensitive ? m_FindText : ToLower(m_FindText);
    const std::vector<std::string> lines = document.editor->GetTextLines();

    for (int lineIndex = 0; lineIndex < static_cast<int>(lines.size()); ++lineIndex) {
        const std::string& line = lines[static_cast<std::size_t>(lineIndex)];
        const std::string haystack = m_FindCaseSensitive ? line : ToLower(line);

        std::size_t offset = 0;
        while (offset <= haystack.size()) {
            const std::size_t found = haystack.find(needle, offset);
            if (found == std::string::npos) {
                break;
            }

            const TextEditor::Coordinates start(lineIndex, ColumnForByteIndex(line, found));
            const TextEditor::Coordinates end(lineIndex,
                                              ColumnForByteIndex(line, found + needle.size()));
            matches.push_back({start, end});
            offset = found + std::max<std::size_t>(1, needle.size());
        }
    }

    return matches;
}

bool ScriptEditorPanel::SelectMatch(Document& document, const Match& match) {
    document.editor->SetSelection(match.start, match.end);
    document.editor->SetCursorPosition(match.end);
    return true;
}

bool ScriptEditorPanel::FindNext(Document& document, bool backwards) {
    if (m_FindText.empty()) {
        return false;
    }

    const std::vector<Match> matches = FindMatches(document);
    if (matches.empty()) {
        SetStatus("No matches for \"" + m_FindText + "\".", false, 2.0);
        return false;
    }

    const TextEditor::Coordinates cursor = document.editor->GetCursorPosition();
    const bool hasSelection = document.editor->HasSelection();

    if (backwards) {
        for (auto it = matches.rbegin(); it != matches.rend(); ++it) {
            const bool before = hasSelection ? it->end < cursor : it->start < cursor;
            if (before) {
                return SelectMatch(document, *it);
            }
        }
        return SelectMatch(document, matches.back());
    }

    for (const Match& match : matches) {
        const bool after = hasSelection ? match.start > cursor : match.start >= cursor;
        if (after) {
            return SelectMatch(document, match);
        }
    }
    return SelectMatch(document, matches.front());
}

bool ScriptEditorPanel::ReplaceCurrent(Document& document) {
    if (m_FindText.empty()) {
        return false;
    }

    bool selectionIsMatch = false;
    if (document.editor->HasSelection()) {
        const std::string selected = document.editor->GetSelectedText();
        selectionIsMatch =
            m_FindCaseSensitive ? selected == m_FindText : ToLower(selected) == ToLower(m_FindText);
    }

    if (!selectionIsMatch && !FindNext(document, false)) {
        return false;
    }

    document.editor->ReplaceSelection(m_ReplaceText);
    document.dirty = true;
    document.syntaxCheckPending = true;
    document.lastEditTime = ImGui::GetTime();
    SetStatus("Replaced 1 occurrence.");
    return true;
}

bool ScriptEditorPanel::ReplaceAll(Document& document) {
    if (m_FindText.empty()) {
        return false;
    }

    const std::vector<Match> matches = FindMatches(document);
    if (matches.empty()) {
        SetStatus("No matches for \"" + m_FindText + "\".", false, 2.0);
        return false;
    }

    for (auto it = matches.rbegin(); it != matches.rend(); ++it) {
        document.editor->SetSelection(it->start, it->end);
        document.editor->SetCursorPosition(it->end);
        document.editor->ReplaceSelection(m_ReplaceText);
    }

    document.dirty = true;
    document.syntaxCheckPending = true;
    document.lastEditTime = ImGui::GetTime();
    SetStatus("Replaced " + std::to_string(matches.size()) + " occurrence(s).");
    return true;
}

} // namespace Editor

namespace Editor {

std::string ScriptEditorPanel::GetTextLine(const Document& document, int line) const {
    if (!document.editor || line < 0 || line >= document.editor->GetTotalLines()) {
        return "";
    }
    const auto lines = document.editor->GetTextLines();
    if (line >= 0 && line < static_cast<int>(lines.size())) {
        return lines[static_cast<std::size_t>(line)];
    }
    return "";
}

} // namespace Editor
