#include "Panels/ConsolePanel.hpp"
#include <imgui.h>

namespace {

struct LineStyle {
    const char* tag;
    ImVec4 color;
};

LineStyle StyleFor(Engine::LogLevel level) {
    switch (level) {
    case Engine::LogLevel::Error:
        return {"[ERROR]", ImVec4(1.0f, 0.35f, 0.35f, 1.0f)};
    case Engine::LogLevel::Debug:
        return {"[DEBUG]", ImVec4(1.0f, 1.0f, 1.0f, 0.5f)};
    case Engine::LogLevel::Info:
        return {"[INFO]", ImVec4(1.0f, 1.0f, 1.0f, 1.0f)};
    }
    return {"[INFO]", ImVec4(1.0f, 1.0f, 1.0f, 1.0f)};
}

} // namespace

namespace Editor {

void ConsolePanel::OnImGuiRender(Engine::Registry& registry, EditorConfig& config) {
    float scrollY = 0.0f;
    float scrollMaxY = 0.0f;
    std::string line;

    (void)registry;
    (void)config;

    if (!ImGui::Begin(m_Name.c_str())) {
        ImGui::End();
        return;
    }

    Engine::GetLogBuffer().CopyEntriesInOrder(m_Entries);

    ImGui::BeginChild("ConsoleScroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
    scrollY = ImGui::GetScrollY();
    scrollMaxY = ImGui::GetScrollMaxY();
    // Stay pinned only while the view is already at the bottom.
    if (scrollMaxY <= 0.0f || scrollY >= scrollMaxY - 1.0f) {
        m_ScrollToBottom = true;
    } else {
        m_ScrollToBottom = false;
    }

    if (m_Entries.empty()) {
        ImGui::TextDisabled("No logs yet.");
    } else {
        for (std::size_t i = 0; i < m_Entries.size(); ++i) {
            Engine::LogEntry const& entry = m_Entries[i];
            LineStyle const style = StyleFor(entry.level);
            line = std::string(style.tag) + " " + entry.message;
            ImGui::TextColored(style.color, "%s", line.c_str());
        }
    }

    if (m_ScrollToBottom) {
        ImGui::SetScrollHereY(1.0f);
    }
    ImGui::EndChild();
    ImGui::End();
}

const std::string& ConsolePanel::GetName() const {
    return m_Name;
}

} // namespace Editor
