#ifndef ENGINE_RESOURCES_SCENEMANAGER_HPP_
#define ENGINE_RESOURCES_SCENEMANAGER_HPP_

#include "ECS/Registry.hpp"
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace Engine {

/**
 * @class SceneManager
 * @brief Switches between scenes while the game runs.
 *
 * A scene is either:
 * - a **code scene**: a C++ function registered with @ref Register that
 *   creates the entities, or
 * - a **scene file** (e.g. @c "Level1.scene") made in the editor. Loading
 *   files needs a loader installed with @ref SetFileLoader. The Engine cannot
 *   depend on Serialization, so Serialization installs it (same pattern as
 *   @ref PrefabSpawner).
 *
 * @ref Load only queues the switch. @ref Application applies it between
 * frames, so no system ever sees a half-loaded world:
 * 1. every system gets @ref System::OnSceneUnloaded,
 * 2. all entities are destroyed and IDs restart at 1 (@ref Registry::ClearEntities),
 * 3. the new scene is built,
 * 4. the @ref OnLoaded callbacks run.
 *
 * Resources (score, lives, settings...) are not touched, so use them for data
 * that must survive a switch.
 *
 * @code
 * auto& scenes = app.GetRegistry().AddResource<Engine::SceneManager>();
 * scenes.Register("menu", [](Engine::Registry& r) { ... });
 * scenes.Load("menu");
 * app.Run();
 * @endcode
 */
class SceneManager {
  public:
    /** @brief Creates the entities of a code scene. */
    using Builder = std::function<void(Registry&)>;
    /** @brief Loads a scene file into the registry. Returns false on failure. */
    using FileLoader = std::function<bool(Registry&, const std::string&)>;
    /** @brief Runs after a scene was built. Receives the scene name. */
    using LoadedCallback = std::function<void(Registry&, const std::string&)>;

    /**
     * @brief Registers a code scene. Replaces any scene with the same name.
     */
    void Register(const std::string& name, Builder builder);

    /** @brief True if @p name was registered with @ref Register. */
    bool IsRegistered(const std::string& name) const;

    /** @brief Installs the function used to load scene files. */
    void SetFileLoader(FileLoader loader);

    /** @brief True once a file loader is installed. */
    bool HasFileLoader() const;

    /**
     * @brief Asks to switch to @p name at the end of the current frame.
     *
     * Code scenes are checked first, then @p name is treated as a scene file
     * path. Calling it again in the same frame replaces the earlier request.
     */
    void Load(const std::string& name);

    /** @brief True while a switch is waiting to be applied. */
    bool HasPendingLoad() const;

    /** @brief Name of the scene currently loaded, or empty if none. */
    const std::string& Current() const;

    /** @brief Adds a callback that runs after every successful switch. */
    void OnLoaded(LoadedCallback callback);

    /**
     * @brief Applies the pending switch, if any. Called by @ref Application.
     *
     * If the scene can't be found (unknown name, no file loader, or missing
     * file), the request is dropped with an error and the current scene keeps
     * running.
     *
     * @return True if a scene was switched.
     */
    bool ApplyPending(Registry& registry);

  private:
    std::unordered_map<std::string, Builder> m_CodeScenes;
    FileLoader m_FileLoader;
    std::vector<LoadedCallback> m_LoadedCallbacks;
    std::optional<std::string> m_Pending;
    std::string m_Current;
};

} // namespace Engine

#endif // ENGINE_RESOURCES_SCENEMANAGER_HPP_
