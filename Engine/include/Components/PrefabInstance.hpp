#ifndef ENGINE_COMPONENTS_PREFABINSTANCE_HPP_
#define ENGINE_COMPONENTS_PREFABINSTANCE_HPP_

#include <string>
#include <utility>

namespace Engine {
/**
 * @struct PrefabInstance
 * @brief Marks the root of a subtree that was saved as or spawned from a `.prefab`.
 *
 * `prefabPath` is project-relative. Preview edits to the sprite path, sprite
 * scale, shape size, and script are copied onto every instance of that file.
 * Position is left where each instance was placed.
 */
struct PrefabInstance {
    std::string prefabPath;

    /**
     * @brief Creates a PrefabInstance.
     * @param path Project-relative path of the `.prefab` file.
     */
    static PrefabInstance Create(std::string path = "") {
        PrefabInstance instance;
        instance.prefabPath = std::move(path);
        return instance;
    }
};
} // namespace Engine

#endif // ENGINE_COMPONENTS_PREFABINSTANCE_HPP_
