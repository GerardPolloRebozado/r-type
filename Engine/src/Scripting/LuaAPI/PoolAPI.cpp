#include "Scripting/LuaAPI/PoolAPI.hpp"

#include "Components/NetworkComponent.hpp"
#include "Components/Opacity.hpp"
#include "Components/Position.hpp"
#include "ECS/Registry.hpp"
#include "Resources/PrefabSpawner.hpp"
#include "Scripting/LuaAPI/Internal/Helpers.hpp"
#include <sol/sol.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace Engine::LuaAPI {

void registerPool(sol::state_view lua, Registry& registry) {
    // Ensure the spawner resource exists (created by Serialization::EnableSceneFiles).
    if (!registry.HasResource<PrefabSpawner>()) {
        registry.AddResource<PrefabSpawner>();
    }
    auto& spawner = registry.GetResource<PrefabSpawner>();

    // Free list keyed by prefab path.
    static std::unordered_map<std::string, std::vector<Entity>> freeList;
    // Entity -> prefab path, so release knows which bucket to use.
    static std::unordered_map<Entity, std::string> entityPath;

    sol::table pool = lua.create_table();

    pool["acquire"] = [&registry, &spawner](const std::string& path, sol::optional<float> x,
                                            sol::optional<float> y) -> Entity {
        Entity entity = static_cast<Entity>(-1);
        auto it = freeList.find(path);
        if (it != freeList.end() && !it->second.empty()) {
            entity = it->second.back();
            it->second.pop_back();
        } else {
            entity = spawner.Spawn(registry, path);
            entityPath[entity] = path;
        }

        if (entity == static_cast<Entity>(-1)) {
            return entity;
        }
        entityPath[entity] = path;

        // Reset transform.
        if (registry.HasComponent<Position>(entity)) {
            auto& pos = registry.RequireComponent<Position>(entity);
            if (x.has_value()) {
                pos.x = *x;
            }
            if (y.has_value()) {
                pos.y = *y;
            }
            pos.prevX = pos.x;
            pos.prevY = pos.y;
            pos.moved = true;
        }
        // Re-enable.
        if (registry.HasComponent<Opacity>(entity)) {
            registry.RequireComponent<Opacity>(entity).alpha = 1.0f;
        }
        return entity;
    };

    pool["release"] = [&registry](Entity entity) -> bool {
        if (!registry.IsEntityAlive(entity)) {
            return false;
        }
        auto pit = entityPath.find(entity);
        if (pit == entityPath.end()) {
            return false;
        }
        const std::string path = pit->second;
        // Hide.
        if (registry.HasComponent<Opacity>(entity)) {
            registry.RequireComponent<Opacity>(entity).alpha = 0.0f;
        }
        freeList[path].push_back(entity);
        return true;
    };

    lua["Pool"] = pool;
}

} // namespace Engine::LuaAPI