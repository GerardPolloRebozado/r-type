#ifndef WAVE_PATTERNS_HPP_
#define WAVE_PATTERNS_HPP_

#include "Components/ColliderComponent.hpp"
#include "Components/Position.hpp"
#include "Components/SpriteComponent.hpp"
#include "ECS/Registry.hpp"
#include "Resources/PrefabSpawner.hpp"
#include <string>
#include <vector>

namespace Engine {

struct EntityBounds {
    float width;
    float height;
    float offsetX; // optional offset from entity origin to bounding box
    float offsetY;
};

enum class WavePatternType {
    Line,
    VFormation,
    Sine,
    Grid,
    Custom // placeholder for future patterns
};

struct WaveConfig {
    WavePatternType pattern;
    std::string prefabPath;
    unsigned int count;           // number of entities
    float spacing;                // distance between entities along pattern axis
    bool attachParent;            // if true, spawn a dummy parent entity and attach children
    std::string parentScriptPath; // optional script for parent entity
    // Additional parameters can be added as needed
};

/**
 * @brief Retrieves the size (width, height) and offset of an entity.
 *
 * The function first checks for a ColliderComponent with a rectangle shape.
 * If none is found, it falls back to the SpriteComponent source rectangle and scale.
 * If both are missing, a default size of 64x64 is returned.
 */
EntityBounds GetEntitySize(Registry& registry, Entity entity);

/**
 * @brief Spawns a wave formation according to the provided configuration.
 *
 * The function creates "count" entities using the PrefabSpawner resource, positions
 * them according to the selected pattern, and optionally creates a parent entity
 * that groups them together.
 *
 * @return Vector of spawned entity IDs.
 */
std::vector<Entity> SpawnWaveFormation(Registry& registry, const WaveConfig& config);

} // namespace Engine
#endif