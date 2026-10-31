#include "Scripting/LuaAPI/WaveAPI.hpp"
#include "Components/Position.hpp"
#include "Resources/PrefabSpawner.hpp"
#include "Scripting/ScriptComponent.hpp"
#include "Utils/WavePatterns.hpp"
#include <algorithm>
#include <utility>

namespace Engine {

void registerWave(sol::state_view lua, Registry& registry) {
    auto parsePattern = [](const std::string& name) -> WavePatternType {
        std::string lower = name;
        std::ranges::transform(lower, lower.begin(), tolower);
        if (lower == "line") {
            return WavePatternType::Line;
        }
        if (lower == "vformation") {
            return WavePatternType::VFormation;
        }
        if (lower == "sine") {
            return WavePatternType::Sine;
        }
        if (lower == "grid") {
            return WavePatternType::Grid;
        }
        return WavePatternType::Custom;
    };

    sol::table waveTable = lua.create_table();

    waveTable.set_function(
        "spawnFormation",
        [lua, &registry, parsePattern](const sol::table& cfg) mutable -> sol::object {
            WaveConfig config{};
            const sol::optional<std::string> prefab = cfg["prefabPath"];
            config.prefabPath = prefab.value_or("");

            const sol::optional<std::string> patternStr = cfg["pattern"];
            config.pattern = parsePattern(patternStr.value_or("line"));

            const sol::optional<unsigned int> count = cfg["count"];
            config.count = count.value_or(1);

            const sol::optional<float> spacing = cfg["spacing"];
            config.spacing = spacing.value_or(64.f);

            const sol::optional<bool> attachParent = cfg["attachParent"];
            config.attachParent = attachParent.value_or(false);

            const sol::optional<std::string> scriptPath = cfg["parentScriptPath"];
            config.parentScriptPath = scriptPath.value_or("");

            auto spawned = SpawnWaveFormation(registry, config);
            sol::table result = lua.create_table();
            for (std::size_t i = 0; i < spawned.size(); ++i) {
                result[i + 1] = spawned[i];
            }
            return sol::make_object(lua, result);
        });

    // Convenience wrappers
    waveTable.set_function(
        "spawnLine", [&](const std::string& prefab, const unsigned int count, const float spacing) {
            const WaveConfig cfg{WavePatternType::Line, prefab, count, spacing, false, ""};
            return SpawnWaveFormation(registry, cfg);
        });
    waveTable.set_function(
        "spawnVFormation", [&](std::string prefab, const unsigned int count, const float spacing) {
            const WaveConfig cfg{
                WavePatternType::VFormation, std::move(prefab), count, spacing, false, ""};
            return SpawnWaveFormation(registry, cfg);
        });
    waveTable.set_function(
        "spawnSine", [&](const std::string& prefab, const unsigned int count, const float spacing) {
            const WaveConfig cfg{WavePatternType::Sine, prefab, count, spacing, false, ""};
            return SpawnWaveFormation(registry, cfg);
        });
    waveTable.set_function(
        "spawnGrid", [&](const std::string& prefab, const unsigned int count, const float spacing) {
            const WaveConfig cfg{WavePatternType::Grid, prefab, count, spacing, false, ""};
            return SpawnWaveFormation(registry, cfg);
        });

    // Utility to get size of an entity
    waveTable.set_function("getEntitySize", [lua, &registry](Entity e) mutable -> sol::table {
        EntityBounds b = GetEntitySize(registry, e);
        sol::table t = lua.create_table();
        t["width"] = b.width;
        t["height"] = b.height;
        t["offsetX"] = b.offsetX;
        t["offsetY"] = b.offsetY;
        return t;
    });

    // Simple prefab spawn helper (optional position)
    sol::table prefabTable = lua.create_table();
    prefabTable.set_function("spawn",
                             [&](std::string path, sol::optional<float> x, sol::optional<float> y) {
                                 auto& spawner = registry.GetResource<PrefabSpawner>();
                                 Entity e = spawner.Spawn(registry, path);
                                 if (x && y) {
                                     if (!registry.HasComponent<Position>(e)) {
                                         registry.AddComponent<Position>(e, Position{*x, *y, 0});
                                     } else {
                                         auto& pos = registry.RequireComponent<Position>(e);
                                         pos.x = *x;
                                         pos.y = *y;
                                         pos.prevX = *x;
                                         pos.prevY = *y;
                                         pos.moved = true;
                                     }
                                 }
                                 return e;
                             });

    lua["Wave"] = waveTable;
    lua["Prefab"] = prefabTable;
}

} // namespace Engine