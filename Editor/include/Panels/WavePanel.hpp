#ifndef EDITOR_PANELS_WAVEPANEL_HPP_
#define EDITOR_PANELS_WAVEPANEL_HPP_

#include "Panels/IEditorPanel.hpp"
#include <extras/IconsFontAwesome6.h>
#include <string>

namespace Editor {

/**
 * @class WavePanel
 * @brief Authors the scene's enemy waves (activation X, prefab, formation, count).
 *
 * The list is stored in EditorConfig/SceneConfig and saved as top-level scene
 * data by SceneSerializer. The runtime WaveSpawnerSystem consumes it, so no
 * Lua spawner script is required.
 */
class WavePanel : public IEditorPanel {
  public:
    WavePanel();
    ~WavePanel() override = default;

    void OnImGuiRender(Engine::Registry& registry, EditorConfig& config) override;
    const std::string& GetName() const override;

    static bool s_IsOpen;

  private:
    std::string m_Name;
};

} // namespace Editor

#endif // EDITOR_PANELS_WAVEPANEL_HPP_
