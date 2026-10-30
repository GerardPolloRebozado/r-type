#ifndef EDITOR_PANELS_SETTINGSPANEL_HPP_
#define EDITOR_PANELS_SETTINGSPANEL_HPP_

#include "Panels/IEditorPanel.hpp"
#include <atomic>
#include <string>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>

namespace Editor {

class SettingsPanel : public IEditorPanel {
  public:
    SettingsPanel() = default;
    ~SettingsPanel() override;

    void OnImGuiRender(Engine::Registry& registry, EditorConfig& config) override;
    const std::string& GetName() const override;

    static std::atomic<float> s_BuildProgress;
    static std::atomic<bool> s_IsBuilding;
    static bool s_IsOpen;

    static void StartBuild(EditorConfig& config, Engine::Registry& registry);

    /**
     * @brief Logs the result of a finished build to the Console (success, or
     * the last lines of the build output on failure). Call every frame from
     * the main thread: the log buffer is not thread-safe, so the build thread
     * only stores its report.
     */
    static void ReportBuildResult();

  private:
    struct BuildReport {
        bool succeeded = false;
        std::string exePath;
        std::vector<std::string> output; // last kBuildOutputLines lines
    };
    static constexpr std::size_t kBuildOutputLines = 40;

    std::string m_Name = "Settings";
    static std::thread s_BuildThread;
    static std::mutex s_BuildMutex;
    static std::optional<BuildReport> s_BuildReport;
};

} // namespace Editor
#endif // EDITOR_PANELS_SETTINGSPANEL_HPP_
