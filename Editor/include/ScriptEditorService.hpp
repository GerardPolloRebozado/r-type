#ifndef EDITOR_SCRIPTEDITORSERVICE_HPP_
#define EDITOR_SCRIPTEDITORSERVICE_HPP_

#include <filesystem>
#include <utility>
#include <vector>

namespace Editor {

struct ScriptOpenRequest {
    std::filesystem::path path;
};

/**
 * @brief Small mailbox used by panels to ask the Script Editor to open files.
 *
 * Registry resources are used as the common meeting point because panels do
 * not hold references to each other. Requests are consumed by the script
 * editor on its next render, which keeps the actual file I/O on the UI thread.
 */
struct FontSizeChangeRequest {
    float fontSize = 15.0f;
};

class ScriptEditorService {
  public:
    void Open(std::filesystem::path path) {
        m_OpenRequests.push_back({std::move(path)});
        m_FocusRequested = true;
    }

    void RequestFocus() {
        m_FocusRequested = true;
    }

    void RequestFontSizeChange(float fontSize) {
        m_FontSizeChangeRequested = true;
        m_FontSize = fontSize;
    }

    std::vector<ScriptOpenRequest> ConsumeOpenRequests() {
        std::vector<ScriptOpenRequest> requests = std::move(m_OpenRequests);
        m_OpenRequests.clear();
        return requests;
    }

    bool ConsumeFocusRequest() {
        const bool requested = m_FocusRequested;
        m_FocusRequested = false;
        return requested;
    }

    std::optional<FontSizeChangeRequest> ConsumeFontSizeChangeRequest() {
        if (m_FontSizeChangeRequested) {
            m_FontSizeChangeRequested = false;
            return FontSizeChangeRequest{m_FontSize};
        }
        return std::nullopt;
    }

  private:
    std::vector<ScriptOpenRequest> m_OpenRequests;
    bool m_FocusRequested = false;
    bool m_FontSizeChangeRequested = false;
    float m_FontSize = 15.0f;
};

} // namespace Editor

#endif // EDITOR_SCRIPTEDITORSERVICE_HPP_
