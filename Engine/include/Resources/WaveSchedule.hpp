#ifndef ENGINE_RESOURCES_WAVESCHEDULE_HPP_
#define ENGINE_RESOURCES_WAVESCHEDULE_HPP_

#include "Utils/WavePatterns.hpp"
#include <string>
#include <vector>

namespace Engine {

/**
 * @struct WaveDefinition
 * @brief One enemy wave configured from the visual editor
 *
 * The runtime @ref WaveSpawnerSystem spawns the wave once the active camera
 * reaches @ref triggerX, then positions the formation at
 * `cameraX + spawnOffsetX` and @ref baseY.
 */
struct WaveDefinition {
    std::string name = "Wave";
    float triggerX = 0.0f;
    std::string prefabPath;
    WavePatternType formation = WavePatternType::Line;
    unsigned int count = 1;
    float spacing = 100.0f;
    float baseY = 0.0f;
    float spawnOffsetX = 900.0f;
    bool attachParent = false;
    std::string groupScriptPath;
};

/**
 * @struct WaveSchedule
 * @brief Registry resource holding the scene's waves and their runtime state.
 *
 * @ref waves is serialized with the scene config; @ref spawned is runtime-only
 * and reset whenever a scene is loaded.
 */
struct WaveSchedule {
    std::vector<WaveDefinition> waves;
    std::vector<unsigned char> spawned;
};

} // namespace Engine

#endif // ENGINE_RESOURCES_WAVESCHEDULE_HPP_