#ifndef ENGINE_RESOURCES_PLAYERINPUTSTATE_HPP_
#define ENGINE_RESOURCES_PLAYERINPUTSTATE_HPP_

#include <cstddef>
#include <unordered_map>

namespace Engine::Multiplayer {

/**
 * @struct NetInput
 * @brief The last input snapshot received for one player.
 * TODO: this is not correct, it should be flexible, obviously
 */
struct NetInput {
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;
    bool shoot = false;
};

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
    std::unordered_map<std::size_t, NetInput> byNetworkId;
};

} // namespace Engine::Multiplayer

#endif // ENGINE_RESOURCES_PLAYERINPUTSTATE_HPP_
