#ifndef ENGINE_RESOURCES_PLAYERSLOTSTATE_HPP_
#define ENGINE_RESOURCES_PLAYERSLOTSTATE_HPP_

#include <cstddef>
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <unordered_set>

namespace Engine::Multiplayer {

/**
 * @struct PlayerSlotState
 * @brief Maps connected players to a color slot (0-based).
 *
 * The server assigns the smallest free slot on connect and frees it on
 * disconnect, so up to @ref kMaxPlayers players always get distinct colors.
 * The slot travels with the player spawn/state messages, so gameplay scripts
 * can query it with Net.playerSlot(entity) on the server and on clients.
 *
 * The R-Type game maps slot N to the sprite block 5*N+1 .. 5*N+5 (see
 * player.lua), which is why the cap is part of this resource.
 */
struct PlayerSlotState {
    /** Maximum number of concurrent players (and color blocks). */
    static constexpr std::uint8_t kMaxPlayers = 5;

    /** network id -> slot index in [0, kMaxPlayers). */
    std::unordered_map<std::size_t, std::uint8_t> byNetworkId;

    /**
     * @brief Server-side: assign the smallest free slot to a player.
     * @return the slot, or std::nullopt when every slot is taken.
     */
    std::optional<std::uint8_t> Assign(std::size_t networkId) {
        std::unordered_set<std::uint8_t> used;
        used.reserve(byNetworkId.size());
        for (const auto& [id, slot] : byNetworkId) {
            used.insert(slot);
        }
        for (std::uint8_t slot = 0; slot < kMaxPlayers; ++slot) {
            if (!used.contains(slot)) {
                byNetworkId[networkId] = slot;
                return slot;
            }
        }
        return std::nullopt;
    }

    /** Frees the slot held by a player (disconnect, despawn). */
    void Release(std::size_t networkId) {
        byNetworkId.erase(networkId);
    }

    /** Slot assigned to a network id, if any. */
    std::optional<std::uint8_t> Get(std::size_t networkId) const {
        const auto it = byNetworkId.find(networkId);
        if (it == byNetworkId.end()) {
            return std::nullopt;
        }
        return it->second;
    }
};

} // namespace Engine::Multiplayer

#endif // ENGINE_RESOURCES_PLAYERSLOTSTATE_HPP_
