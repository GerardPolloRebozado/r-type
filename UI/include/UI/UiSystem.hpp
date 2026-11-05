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
     * @brief Draws every alive UI entity. Called from the UiPass, not Render.
     */
    void Draw(Registry& registry);

  private:
    void LoadScript(Registry& registry, Entity entity);
    void UnloadScript(Registry& registry, Entity entity);

    static std::unordered_set<std::string> s_WarnedMissing;
};

} // namespace Engine

#endif // UI_UISYSTEM_HPP_
