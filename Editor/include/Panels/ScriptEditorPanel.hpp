#ifndef EDITOR_PANELS_SCRIPTEDITORPANEL_HPP_
#define EDITOR_PANELS_SCRIPTEDITORPANEL_HPP_

#include "Panels/IEditorPanel.hpp"
#include "TextEditor.h"

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Editor {

class ScriptEditorPanel : public IEditorPanel {
  public:
    ScriptEditorPanel();
    ~ScriptEditorPanel() override;

    void OnImGuiRender(Engine::Registry& registry, EditorConfig& config) override;
    const std::string& GetName() const override;

    /** @brief Saves every dirty document. Returns false if at least one save failed. */
    bool SaveAll();
    /** @brief True when at least one document has unsaved edits. */
    bool HasDirtyDocuments() const;
    /** @brief Brings the panel to the front on its next frame. */
    void Focus();

  private:
    struct Document {
        std::filesystem::path path;
        std::string displayPath;
        std::unique_ptr<TextEditor> editor;
        std::filesystem::file_time_type modifiedTime{};
        std::uintmax_t fileSize = 0;
        std::string lineEnding = "\n";
        bool utf8Bom = false;
        bool dirty = false;
        bool missing = false;
        bool externalChanged = false;
        bool syntaxCheckPending = false;
        std::size_t errorCount = 0;
        double lastEditTime = 0.0;
        double lastExternalCheck = 0.0;
        float fontSize = 15.0f; // Zoomable font size
    };

    struct Match {
        TextEditor::Coordinates start;
        TextEditor::Coordinates end;
    };

    void ConsumeOpenRequests(Engine::Registry& registry, EditorConfig& config);
    void OpenFile(const std::filesystem::path& requested, EditorConfig& config);
    bool LoadDocument(Document& document);
    bool SaveDocument(Document& document);
    bool SaveDocumentAt(int index);
    void ReloadDocument(Document& document);
    void CloseDocument(int index);
    void RequestClose(int index);
    void ProcessCloseRequest();

    void DrawToolbar();
    void DrawTabs();
    void DrawDocument(Document& document);
    void DrawFindBar(Document& document);
    void DrawStatusBar(const Document* document);
    void DrawClosePopup();
    void DrawGoToLinePopup();

    void HandleShortcuts(Document& document);
    void UpdateDocumentState(Document& document);
    void CheckSyntax(Document& document);
    void PollExternalChange(Document& document);
    void SetStatus(std::string message, bool isError = false, double seconds = 4.0);
    void RefreshFileMetadata(Document& document);
    void ToggleLineComments(Document& document);
    void DuplicateLine(Document& document);
    void NewFile();
    std::string GetTextLine(const Document& document, int line) const;

    std::vector<Match> FindMatches(const Document& document) const;
    bool SelectMatch(Document& document, const Match& match);
    bool FindNext(Document& document, bool backwards);
    bool ReplaceCurrent(Document& document);
    bool ReplaceAll(Document& document);

    std::string m_Name = "Script Editor";
    std::vector<std::unique_ptr<Document>> m_Documents;
    int m_ActiveDocument = -1;

    bool m_FocusRequested = false;
    bool m_ShowWhitespace = false;

    bool m_FindOpen = false;
    bool m_ReplaceOpen = false;
    bool m_FindFocusRequested = false;
    bool m_FindCaseSensitive = false;
    std::string m_FindText;
    std::string m_ReplaceText;

    bool m_GoToLineOpen = false;
    bool m_GoToLineFocusRequested = false;
    int m_GoToLine = 1;

    int m_PendingClose = -1;
    bool m_ClosePopupRequested = false;
    int m_TabCloseRequest = -1;

    std::string m_StatusMessage;
    bool m_StatusIsError = false;
    double m_StatusUntil = 0.0;
};

} // namespace Editor

#endif // EDITOR_PANELS_SCRIPTEDITORPANEL_HPP_
