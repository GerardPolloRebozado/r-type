#ifndef ENGINE_RESOURCES_PLAYERINPUTSTATE_HPP_
#define ENGINE_RESOURCES_PLAYERINPUTSTATE_HPP_

#include "Inputs/IController.hpp"
#include <cstddef>
#include <cstdint>
#include <set>
#include <unordered_map>

namespace Engine::Multiplayer {
/**
 * @struct PlayerInputState
 * @brief Server-side store of the latest input for every connected player.
 *
 * Populated by ReplicationSystem's player_input handler and read by gameplay
 * scripts through Net.input(entity). Keyed by network id so it survives the
 * server replacing the placeholder entity created on connect with the
 * prefab-spawned player.
 */
struct PlayerInputState {
    std::unordered_map<std::size_t, std::set<Action>> byNetworkId;
    std::unordered_map<std::size_t, std::uint32_t> lastSeq;
    std::unordered_map<Action, ScriptComponent> keyHandlers;
};
} // namespace Engine::Multiplayer

#endif // ENGINE_RESOURCES_PLAYERINPUTSTATE_HPP_