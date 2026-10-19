#ifndef ENGINE_RESOURCES_PREFABSPAWNER_HPP_
#define ENGINE_RESOURCES_PREFABSPAWNER_HPP_

#include "ECS/Entity.hpp"
#include "ECS/Registry.hpp"
#include <functional>
#include <string>

namespace Engine {

/**
 * @struct PrefabSpawner
 * @brief Runtime prefab instantiation hook.
 *
 * The Engine module cannot depend on the Serialization module (Serialization
 * already depends on Engine), so prefab loading is installed here as a
 * callback. Serialization::SceneSerializer registers the implementation when it
 * deserializes a scene, which means every editor-generated game gets runtime
 * prefab spawning for free.
 *
 * Engine systems (notably ReplicationSystem) call Spawn() when they need to
 * instantiate a prefab during play.
 */
struct PrefabSpawner {
    /** @brief Spawns every entity in the prefab and returns its root, or (Entity)-1. */
    std::function<Entity(Registry&, const std::string&)> Spawn;
};

} // namespace Engine

#endif // ENGINE_RESOURCES_PREFABSPAWNER_HPP_
