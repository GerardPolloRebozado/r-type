#ifndef EDITOR_PANELS_PREVIEWPANEL_HPP_
#define EDITOR_PANELS_PREVIEWPANEL_HPP_

#include "Panels/IEditorPanel.hpp"
#include "PluginEditor.hpp"
#include <memory>
#include <raylib-cpp.hpp>
#include <string>
#include <vector>

namespace Editor {

/**
 * @brief Asset preview. Images are drawn as textures. A `.prefab` is edited here.
 *
 * Prefab mode shows the root components, then each child level. Save writes the
 * file and copies those components onto every scene instance of that prefab.
 * The root position stays where each instance was placed. A child position is
 * applied as an offset from its parent.
 */
class PreviewPanel : public IEditorPanel {
  public:
    /** @brief Builds the component editors used by the prefab view. */
    PreviewPanel();
    /** @brief Releases the prefab preview registry. */
    ~PreviewPanel() override;

    /**
     * @brief Draws the preview window for the asset selected in the Assets panel.
     * @param registry Scene registry. Prefab saves update instances stored here.
     * @param config Editor config. `m_SelectedAssetPath` chooses the file.
     */
    void OnImGuiRender(Engine::Registry& registry, EditorConfig& config) override;

    /**
     * @brief Returns the dock name of this panel.
     * @return `"Preview"`.
     */
    const std::string& GetName() const override;

  private:
    /** @brief Prefab file loaded into its own registry for editing. */
    struct PrefabPreviewState;

    /**
     * @brief Draws one `.prefab`: a fixed Save bar and the scrolling editors.
     * @param scene Scene that owns the instances to update on Save.
     * @param config Editor config used to resolve the project folder.
     * @param absolutePath Absolute path of the `.prefab` file.
     */
    void RenderPrefab(Engine::Registry& scene, EditorConfig& config,
                      const std::string& absolutePath);

    /**
     * @brief Loads `absolutePath` into `m_Prefab` when it is not already loaded.
     * @param scene Scene used to resolve the project-relative path.
     * @param config Editor config. Supplies the project folder when the scene has none.
     * @param absolutePath Absolute path of the `.prefab` file.
     */
    void EnsurePrefabLoaded(Engine::Registry& scene, EditorConfig& config,
                            const std::string& absolutePath);

    /**
     * @brief Draws the root name, its components, and the nested children.
     *
     * Right-clicking empty space opens the menu that adds a component to the root.
     */
    void DrawPrefabControls();

    /**
     * @brief Draws every component editor attached to one prefab entity.
     * @param entity Entity inside the prefab preview registry.
     */
    void DrawPrefabEntityEditors(Engine::Entity entity);

    /**
     * @brief Draws the direct children of `parent`, then their children.
     *
     * Each level is labeled `Children of <name> (level N)`. A child is a tree
     * row, so it is not drawn like a component header. Right-clicking a row
     * adds a component to that child.
     * @param parent Entity whose direct children are listed.
     * @param level Depth of those children. The root's children are level 1.
     */
    void DrawPrefabChildren(Engine::Entity parent, int level);

    /**
     * @brief Writes the open prefab and applies it to every linked scene instance.
     *
     * A component cleared in the preview is restored from the file, so an
     * instance keeps it. Child positions are stored as offsets from the parent.
     * @param scene Scene whose `PrefabInstance` roots are updated.
     */
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
