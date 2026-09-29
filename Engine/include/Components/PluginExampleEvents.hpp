#ifndef ENGINE_COMPONENTS_PLUGINEXAMPLEEVENTS_HPP_
#define ENGINE_COMPONENTS_PLUGINEXAMPLEEVENTS_HPP_

#include "ECS/Entity.hpp"
#include <functional>
#include <vector>

namespace Engine {
/**
 * @brief Holds user-defined callbacks (hooks) for this specific entity.
 * Functions exactly like Godot Signals or Unity Events.
 */
struct PluginExampleEvents {
    // A list of callbacks triggered when a specific condition is met
    std::vector<std::function<void(Entity me)>> onConditionMet;

    // Another list of callbacks that passes data (like the new state)
    std::vector<std::function<void(Entity me, int newState)>> onStateChanged;
};
} // namespace Engine

#endif // ENGINE_COMPONENTS_PLUGINEXAMPLEEVENTS_HPP_
