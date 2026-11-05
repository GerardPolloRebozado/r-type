#ifndef UI_UIPLUGIN_HPP_
#define UI_UIPLUGIN_HPP_

#include "Plugins/IPlugin.hpp"
#include "UI/UiComponent.hpp"

namespace Engine {

/**
 * @brief Registers the Lua UI component, its draw pass, and the ImGui Lua API.
 */
class UiPlugin : public IPlugin {
  public:
    void Init(Registry& registry) override;

    /**
     * @brief Attaches a UI script to an entity, registering the component type if needed.
     */
    static void AttachToEntity(Registry& registry, Entity entity, const UiComponent& component);
};

} // namespace Engine

#endif // UI_UIPLUGIN_HPP_
