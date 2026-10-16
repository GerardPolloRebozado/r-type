/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** ConnectionsPluginEditor.hpp
*/
#ifndef G_ING_500_BAR_5_1_RTYPE_1_CONNECTIONSPLUGINEDITOR_HPP
#define G_ING_500_BAR_5_1_RTYPE_1_CONNECTIONSPLUGINEDITOR_HPP
#include "Components/SignalConnections.hpp"
#include "PluginEditor.hpp"
#include "Scripting/SignalDispatch.hpp"

#include <string>

namespace Editor {

/**
 * @class ConnectionsPluginEditor
 * @brief Inspector UI for Godot-style signal connections: which signal of the
 *        selected emitter calls which method on which target entity.
 *
 * Renders one tab per signal provider plugin (see
 * Engine::Signals::GetSignalProviders); new emitter systems appear as new
 * tabs with no editor changes.
 */
class ConnectionsPluginEditor : public IPluginEditor {
  public:
    const char* GetName() const override;

    const char* GetIcon() const override;

    bool OnImGuiRender(Engine::Entity entity, Engine::Registry& registry) override;

    /**
     * @brief Renders just the per-provider tab bar (no collapsing header).
     * @return False when the entity lacks the component (nothing drawn).
     *
     * Shared by the Inspector section and the standalone Connections panel
     * so both stay in sync. Draft state (signal/target/method) is shared;
     * only one entity is selected at a time.
     */
    bool RenderTabs(Engine::Entity entity, Engine::Registry& registry);

    bool HasComponent(Engine::Entity entity, Engine::Registry& registry) override;

    void AddComponent(Engine::Entity entity, Engine::Registry& registry) override;

    void RemoveComponent(Engine::Entity entity, Engine::Registry& registry) override;

  private:
    void RenderProviderTab(Engine::Entity entity, Engine::Registry& registry,
                           Engine::SignalConnections& stored,
                           const Engine::Signals::SignalProvider& provider, int tabIndex);

    int m_SignalIdx = 0;
    Engine::Entity m_Target = static_cast<Engine::Entity>(-1);
    std::string m_Method;
};

} // namespace Editor
#endif // G_ING_500_BAR_5_1_RTYPE_1_CONNECTIONSPLUGINEDITOR_HPP
