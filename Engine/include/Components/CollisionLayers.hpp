#ifndef ENGINE_COMPONENTS_COLLISIONLAYERS_HPP_
#define ENGINE_COMPONENTS_COLLISIONLAYERS_HPP_

#include <array>
#include <cstdint>
#include <string>
#include <string_view>

namespace Engine {
/**
 * @brief Namespace of collision layer helpers shared by @ref ColliderComponent,
 *        @ref CollisionSystem, the editor Inspector and the Lua bindings.
 *
 * A collider belongs to one @ref ColliderComponent::layer and carries a
 * @ref ColliderComponent::mask of the layers it is willing to touch. Two
 * colliders only interact when **both** masks contain the other's layer, so
 * either side can veto a pair ("this only collides with walls and pickups").
 *
 * Layers are plain integers in [0, @ref kLayerCount); nothing enforces the
 * names below, they are a convention so the Inspector and Lua scripts can talk
 * about layers without magic numbers.
 */
namespace CollisionLayers {

/** @brief Number of usable layers (one bit each in a 32-bit mask). */
inline constexpr int kLayerCount = 32;

/** @brief Mask value matching every layer: the default for new colliders. */
inline constexpr std::uint32_t kAllLayers = 0xFFFFFFFFu;

/** @brief Mask value matching nothing: use it to make a collider inert. */
inline constexpr std::uint32_t kNoLayers = 0u;

/** @brief Layer every new collider starts on (the `Default` preset). */
inline constexpr int kDefaultLayer = 0;

/**
 * @brief Conventional names for the first layers, used by the Inspector and
 *        `Layers.name()` in Lua. Layers past these are named "Layer N". (these can be changed, or
 *        directly removed and provide the option to modify them
 */
inline constexpr std::array<std::string_view, 8> kLayerNames = {
    "Default", "Player", "Enemy", "PlayerBullet", "EnemyBullet", "Wall", "Pickup", "Trigger"};

/** @brief Number of entries in @ref kLayerNames. */
inline constexpr int kNamedLayerCount = static_cast<int>(kLayerNames.size());

/**
 * @brief The single-bit mask of one layer.
 * @return 0 for out-of-range layers, so callers never shift by >= 32.
 */
inline std::uint32_t Bit(int layer) {
    if (layer < 0 || layer >= kLayerCount) {
        return 0u;
    }
    return 1u << layer;
}

/**
 * @brief Display name of a layer: the preset name, or "Layer N" past them.
 */
inline std::string Name(int layer) {
    if (layer >= 0 && layer < kNamedLayerCount) {
        return std::string(kLayerNames[static_cast<std::size_t>(layer)]);
    }
    return "Layer " + std::to_string(layer);
}

/**
 * @brief Resolves a layer from its name or its number.
 * @return The layer index, or -1 when @p name matches nothing.
 */
inline int Index(std::string_view name) {
    for (int layer = 0; layer < kNamedLayerCount; ++layer) {
        if (kLayerNames[static_cast<std::size_t>(layer)] == name) {
            return layer;
        }
    }
    // Numeric fallback: "3" or "Layer 3".
    const std::size_t first = name.find_first_of("0123456789");
    if (first == std::string_view::npos) {
        return -1;
    }
    int value = 0;
    for (std::size_t i = first; i < name.size(); ++i) {
        if (name[i] < '0' || name[i] > '9') {
            break;
        }
        value = value * 10 + (name[i] - '0');
        if (value >= kLayerCount) {
            return -1;
        }
    }
    return value;
}

} // namespace CollisionLayers
} // namespace Engine

#endif // ENGINE_COMPONENTS_COLLISIONLAYERS_HPP_
