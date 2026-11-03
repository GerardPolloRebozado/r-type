#ifndef ENGINE_SCRIPTING_SCRIPTVARIABLESYNC_HPP_
#define ENGINE_SCRIPTING_SCRIPTVARIABLESYNC_HPP_

#include "ECS/Entity.hpp"

#include <cstddef>
#include <glaze/glaze.hpp>
#include <string>
#include <vector>

namespace Engine {
class Registry;
}

namespace Engine::Multiplayer {

/**
 * @brief Reads registered fields from an entity's loaded script table.
 * @return true when the entity has a loaded ScriptComponent, even if some
 *         fields were missing (missing fields are omitted from @p out).
 */
bool ReadScriptVariables(Registry& registry, Entity entity, const std::vector<std::string>& fields,
                         glz::generic::object_t& out);

/**
 * @brief Writes replicated fields into an entity's loaded script table.
 * @return true when the table was available and updated.
 */
bool WriteScriptVariables(Registry& registry, Entity entity, const glz::generic::object_t& vars);

/**
 * @brief Applies a replicated "vars" object on a client.
 *
 * Writes immediately when the script has registered; otherwise keeps the
 * values pending until Net.sync runs during the script's onCreate.
 */
void ApplyReplicatedScriptVariables(Registry& registry, std::size_t networkId, Entity entity,
                                    const glz::generic::object_t& vars);

/**
 * @brief Applies pending values after a script registers with Net.sync.
 */
void ResolvePendingScriptVariables(Registry& registry, Entity entity, std::size_t networkId);

} // namespace Engine::Multiplayer

#endif // ENGINE_SCRIPTING_SCRIPTVARIABLESYNC_HPP_
