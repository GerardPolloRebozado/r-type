#ifndef EDITOR_PANELS_PREVIEWPANEL_HPP_
#define EDITOR_PANELS_PREVIEWPANEL_HPP_

#include "Panels/IEditorPanel.hpp"
#include "PluginEditor.hpp"
#include <memory>
#include <raylib-cpp.hpp>
#include <string>
#include <vector>

namespace Editor {

class PreviewPanel : public IEditorPanel {
  public:
    PreviewPanel();
    ~PreviewPanel() override;

    void OnImGuiRender(Engine::Registry& registry, EditorConfig& config) override;
    const std::string& GetName() const override;

  private:
    struct PrefabPreviewState;

    void RenderPrefab(Engine::Registry& scene, EditorConfig& config, const std::string& absolutePath);
    void EnsurePrefabLoaded(Engine::Registry& scene, EditorConfig& config,
                            const std::string& absolutePath);
    void DrawPrefabControls();
    void SyncPrefabEdits(Engine::Registry& scene);

    std::string m_Name = "Preview";
    std::string m_CurrentPreviewPath;
    raylib::Texture2D m_PreviewTexture;
    bool m_IsOpen = false;
    std::vector<std::unique_ptr<IPluginEditor>> m_PluginEditors;
    std::unique_ptr<PrefabPreviewState> m_Prefab;
};

} // namespace Editor
#endif // EDITOR_PANELS_PREVIEWPANEL_HPP_
