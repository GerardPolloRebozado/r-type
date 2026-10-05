#ifndef EDITOR_PANELS_SETTINGSPANEL_HPP_
#define EDITOR_PANELS_SETTINGSPANEL_HPP_

#include "Panels/IEditorPanel.hpp"
#include <atomic>
#include <string>
#include <thread>

namespace Editor {

class SettingsPanel : public IEditorPanel {
  public:
    SettingsPanel() = default;
    ~SettingsPanel() override;

    void OnImGuiRender(Engine::Registry* registry, EditorConfig* config) override;
    const std::string& GetName() const override;

    static std::atomic<float> s_BuildProgress;
    static std::atomic<bool> s_IsBuilding;
    static bool s_IsOpen;

    static void StartBuild(EditorConfig* config, Engine::Registry* registry);

  private:
    std::string m_Name = "Settings";
    static std::thread s_BuildThread;
};

} // namespace Editor
#endif // EDITOR_PANELS_SETTINGSPANEL_HPP_
