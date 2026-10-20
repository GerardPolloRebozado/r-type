#include "RuntimePrefab.hpp"

#include "Assets/ProjectPaths.hpp"
#include "GlazeSerializer.hpp"
#include "Logging/Logger.hpp"
#include "Resources/PrefabSpawner.hpp"

#include <filesystem>

namespace Serialization {

Engine::Entity SpawnPrefabRuntime(Engine::Registry& registry, const std::string& prefabPath) {
    constexpr Engine::Entity kInvalid = static_cast<Engine::Entity>(-1);
    if (prefabPath.empty()) {
        return kInvalid;
    }

    const std::filesystem::path resolved = Engine::ResolveAssetPath(registry, prefabPath);
    if (!std::filesystem::exists(resolved)) {
        log_error("[Prefab] Prefab not found: " + resolved.string());
        return kInvalid;
    }

    GlazeSerializer serializer;
    GlazeSerializer::SceneSnapshot snapshot;
    std::string error;
    if (!serializer.DeserializeFromFile(resolved.string(), snapshot, &error)) {
        log_error("[Prefab] Failed to read " + resolved.string() +
                  (error.empty() ? "" : " (" + error + ")"));
        return kInvalid;
    }
    if (snapshot.entities.empty()) {
        log_error("[Prefab] Prefab has no entities: " + resolved.string());
        return kInvalid;
    }

    return serializer.SpawnEntities(registry, snapshot.entities);
}

void RegisterRuntimePrefabSpawner(Engine::Registry& registry) {
    if (registry.HasResource<Engine::PrefabSpawner>()) {
        return;
    }
    auto& spawner = registry.AddResource<Engine::PrefabSpawner>();
    spawner.Spawn = [](Engine::Registry& reg, const std::string& path) {
        return SpawnPrefabRuntime(reg, path);
    };
}

} // namespace Serialization
