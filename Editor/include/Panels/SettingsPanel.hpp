#ifndef EDITOR_PANELS_SETTINGSPANEL_HPP_
#define EDITOR_PANELS_SETTINGSPANEL_HPP_

/**
 * @file SettingsPanel.hpp
 * @brief Declares project settings and standalone build generation.
 */

#include "Panels/IEditorPanel.hpp"
#include <atomic>
#include <string>
#include <thread>

namespace Editor {

/**
 * @class SettingsPanel
 * @brief Edits project settings and generates/builds a standalone game.
 */
class SettingsPanel : public IEditorPanel {
  public:
    SettingsPanel() = default;
    ~SettingsPanel() override;

    /**
     * @brief Draws project settings and build controls.
     */
    void OnImGuiRender(Engine::Registry& registry, EditorConfig& config) override;

    /**
     * @brief Returns the panel name used for docking.
     */
    const std::string& GetName() const override;

    /** @brief Current build progress in [0,1]. */
    static std::atomic<float> s_BuildProgress;

    /** @brief True while the background build is running. */
    static std::atomic<bool> s_IsBuilding;

    /** @brief True when the settings panel is visible. */
    static bool s_IsOpen;

    /**
     * @brief Starts a standalone build for the given project configuration.
     * @param config Project configuration.
     * @param registry Registry used to gather scene data.
     */
    static void StartBuild(EditorConfig& config, Engine::Registry& registry);

  private:
    std::string m_Name = "Settings";

    /** @brief Background build thread owned by the settings panel. */
    static std::thread s_BuildThread;
};

} // namespace Editor
#endif // EDITOR_PANELS_SETTINGSPANEL_HPP_