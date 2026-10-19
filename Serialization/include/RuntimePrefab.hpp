#ifndef SERIALIZATION_RUNTIMEPREFAB_HPP_
#define SERIALIZATION_RUNTIMEPREFAB_HPP_

#include "ECS/Entity.hpp"
#include "ECS/Registry.hpp"
#include <string>

namespace Serialization {

/**
 * @brief Instantiates a .prefab file at runtime.
 *
 * The prefab stores the same EntitySnapshot format as a scene subtree, so it
 * can be loaded with the scene serializer and spawned through SpawnEntities.
 * Paths are project-root relative (e.g. "Prefabs/Player.prefab").
 *
 * @return The prefab root entity, or (Entity)-1 on failure.
 */
Engine::Entity SpawnPrefabRuntime(Engine::Registry& registry, const std::string& prefabPath);

/**
 * @brief Installs SpawnPrefabRuntime as Engine::PrefabSpawner in the registry.
 *
 * Called automatically by SceneSerializer::Deserialize so editor-generated
 * games get runtime prefab spawning without extra setup.
 */
void RegisterRuntimePrefabSpawner(Engine::Registry& registry);

} // namespace Serialization

#endif // SERIALIZATION_RUNTIMEPREFAB_HPP_
