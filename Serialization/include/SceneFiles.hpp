#ifndef SERIALIZATION_SCENEFILES_HPP_
#define SERIALIZATION_SCENEFILES_HPP_

#include "ECS/Registry.hpp"
#include "Resources/SceneManager.hpp"
#include <string>
#include <vector>

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

/**
 * @brief Picks the scene a game starts with.
 *
 * A command-line argument ending in @c .scene wins, so the editor (or you)
 * can start the game on any scene: @c "./Game Level2.scene". Otherwise
 * @p fallback is used: the project's startup scene.
 *
 * @param args Command-line arguments without the program name.
 * @param fallback Scene to use when no argument names one.
 */
std::string PickStartupScene(const std::vector<std::string>& args, const std::string& fallback);

} // namespace Serialization

#endif // SERIALIZATION_SCENEFILES_HPP_
