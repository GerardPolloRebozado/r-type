#ifndef EDITOR_ENTITYSUBTREE_HPP_
#define EDITOR_ENTITYSUBTREE_HPP_

#include "ECS/Registry.hpp"
#include "GlazeSerializer.hpp"
#include <filesystem>
#include <string>
#include <vector>

namespace Editor {
/**
 * @brief Copies an entity plus its children, and writes or spawns `.prefab` files.
 *
 * A paste or a prefab spawn is a new subtree. Positions stay the same, so the
 * copy overlaps the original until it is moved. A Name that is already used in
 * the scene becomes `name(1)`, then `name(2)`. Parent links, signal targets,
 * and camera follow targets that point inside the copied subtree are remapped
 * to the new ids. Links that point outside the subtree stay as they are.
 */
class EntitySubtree {
  public:
    /**
     * @brief Stores the entity and every descendant for a later Paste.
     * @param registry Registry that owns the entity.
     * @param root Entity to copy. A dead id leaves the previous copy in place.
     */
    static void Copy(Engine::Registry& registry, Engine::Entity root);

    /**
     * @brief Returns whether Copy has stored a subtree.
     */
    static bool CanPaste();

    /**
     * @brief Spawns the stored subtree and returns its new root.
     * @return The new root, or `(Entity)-1` when the clipboard is empty.
     */
    static Engine::Entity Paste(Engine::Registry& registry);

    /**
     * @brief Writes `<Name>.prefab` in `directory` and marks `root` with that path.
     * @return The project-relative path, or an empty string when the write fails.
     *
     * An existing file is left untouched. The next name is `Name_2.prefab`, then
     * `Name_3.prefab`. The stored path is a marker. Preview edits to the sprite
     * path, sizes, and script are applied to this entity later; position is not.
     */
    static std::string SavePrefab(Engine::Registry& registry, Engine::Entity root,
                                   const std::filesystem::path& directory);

    /**
     * @brief Spawns the subtree stored in a `.prefab` and marks the new root.
     * @param prefabPath Project-relative or absolute path of the file.
     * @return The new root, or `(Entity)-1` when the file cannot be read.
     */
    static Engine::Entity SpawnPrefab(Engine::Registry& registry, const std::string& prefabPath);

    /**
     * @brief Reads a `.prefab` file into subtree snapshots.
     * @return False when the file is missing or empty.
     */
    static bool ReadPrefabFile(
        const std::string& absolutePath,
        std::vector<Serialization::GlazeSerializer::EntitySnapshot>& snapshots);

    /**
     * @brief Writes subtree snapshots as a `.prefab` file.
     */
    static bool WritePrefabFile(
        const std::string& absolutePath,
        const std::vector<Serialization::GlazeSerializer::EntitySnapshot>& snapshots);
};
} // namespace Editor

#endif // EDITOR_ENTITYSUBTREE_HPP_
