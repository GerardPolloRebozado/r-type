#ifndef SERIALIZATION_SCENEFILES_HPP_
#define SERIALIZATION_SCENEFILES_HPP_

#include "ECS/Registry.hpp"
#include "Resources/SceneManager.hpp"

namespace Serialization {

/**
 * @brief Lets the game's SceneManager load @c .scene files made in the editor.
 *
 * Adds the @ref Engine::SceneManager resource if the registry doesn't have one
 * yet, then installs a file loader that calls @ref SceneSerializer::Deserialize.
 * After this, @c scenes.Load("Level1.scene") (or @c Scene.load in Lua) works
 * alongside code scenes registered with @ref Engine::SceneManager::Register.
 *
 * Only the entities are loaded. Project settings stored in the scene file
 * (window title, input map...) are ignored, since the game already set them up.
 *
 * @return The scene manager, for chaining @c Load().
 */
Engine::SceneManager& EnableSceneFiles(Engine::Registry& registry);

} // namespace Serialization

#endif // SERIALIZATION_SCENEFILES_HPP_
