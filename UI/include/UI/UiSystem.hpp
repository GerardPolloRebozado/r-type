#ifndef UI_UISYSTEM_HPP_
#define UI_UISYSTEM_HPP_

#include "ECS/System.hpp"
#include <string>
#include <unordered_set>

namespace Engine {

/**
 * @brief Loads UiComponent scripts and calls draw inside the ImGui frame.
 */
class UiSystem : public System {
  public:
    void OnSceneUnloaded(Registry& registry) override;

    /**
     * @brief Draws the current page, then standalone widgets. Called from the UiPass.
     *
     * The first draw opens the start page (UiNavigation). Widgets parented to a
     * screen are drawn by that screen. Custom scripts draw every frame.
     */
    void Draw(Registry& registry);

    /**
     * @brief Draws one widget template into the current ImGui window.
     *
     * Screens are skipped. The editor uses this so a placed widget is visible
     * without starting a second ImGui frame.
     */
    void DrawInPlace(Registry& registry, Entity entity);

    /**
     * @brief Editor: draws the screen of that page and its children inside the rect.
     *
     * Uses the same code as the game, in preview mode. Click actions do not run
     * while EditorMode is present.
     */
    void DrawPagePreview(Registry& registry, const std::string& page, float x, float y, float width,
                         float height);

  private:
    void LoadScript(Registry& registry, Entity entity);
    void UnloadScript(Registry& registry, Entity entity);

    static std::unordered_set<std::string> s_WarnedMissing;
};

} // namespace Engine

#endif // UI_UISYSTEM_HPP_
