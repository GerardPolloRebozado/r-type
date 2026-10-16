#ifndef ENGINE_SCRIPTING_SIGNALDISPATCH_HPP_
#define ENGINE_SCRIPTING_SIGNALDISPATCH_HPP_

#include "ECS/Entity.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace Engine {
class Registry;
}

/**
 * @brief Godot-style signal dispatch shared by every emitter system.
 *
 * Looks up @ref SignalConnections on @p emitter, and for each connection whose
 * signal matches, calls `method(targetTable, emitter, other)` on the target's
 * loaded script (same self-passing convention as ScriptSystem::Update).
 * Dead targets, missing scripts, and missing methods are skipped (warn-once);
 * Lua errors are logged, never propagated, so one bad handler cannot break
 * the emitting system.
 */
namespace Engine::Signals {

void EmitSignal(Registry& registry, Entity emitter, const std::string& signal, Entity other);

/**
 * @struct SignalProvider
 * @brief One emitting plugin and the signals it can fire.
 *
 * The editor renders one Connections tab per provider. When a new emitter
 * system is added (input actions, timers, script-defined signals), append its
 * entry here and a tab appears automatically — no editor changes needed.
 */
struct SignalProvider {
    std::string_view plugin;
    std::vector<std::string_view> signals;
};

/**
 * @brief All known signal providers, in tab order.
 */
std::vector<SignalProvider> GetSignalProviders();

} // namespace Engine::Signals

#endif // ENGINE_SCRIPTING_SIGNALDISPATCH_HPP_
