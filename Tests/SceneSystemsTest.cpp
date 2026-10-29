// Checks that systems drop their own per-entity state on a scene switch:
// scripts get onDestroy / a fresh onCreate, collision contacts are forgotten.
#include "Assets/ProjectPaths.hpp"
#include "Components/ShapeComponent.hpp"
#include "ECS/Registry.hpp"
#include "Logging/LogBuffer.hpp"
#include "Plugins/ColliderPlugin.hpp"
#include "Resources/SceneManager.hpp"
#include "Scripting/ScriptComponent.hpp"
#include "Scripting/ScriptPlugin.hpp"
#include "Systems/CollisionSystem.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

static int failures = 0;
#define CHECK(cond)                                                                                \
    do {                                                                                           \
        if (!(cond)) {                                                                             \
            std::cerr << "FAIL line " << __LINE__ << ": " #cond "\n";                              \
            ++failures;                                                                            \
        }                                                                                          \
    } while (0)

namespace {
namespace fs = std::filesystem;

// Same as one Application frame: update every system, flush, switch scene.
void RunFrame(Engine::Registry& registry) {
    const auto& systems = registry.GetSystems();
    for (const auto& id : registry.GetSystemUpdateOrder()) {
        systems.at(id)->Update(registry, 1.0f / 60.0f);
    }
    registry.FlushQueues();
    registry.GetResource<Engine::SceneManager>().ApplyPending(registry);
}

// Script lines tagged "[T]" written since the last call.
std::vector<std::string> TakeScriptLines(std::size_t& seen) {
    std::vector<Engine::LogEntry> entries;
    Engine::GetLogBuffer().CopyEntriesInOrder(entries);
    std::vector<std::string> lines;
    for (std::size_t i = seen; i < entries.size(); ++i) {
        const auto pos = entries[i].message.find("[T] ");
        if (pos != std::string::npos) {
            lines.push_back(entries[i].message.substr(pos + 4));
        }
    }
    seen = entries.size();
    return lines;
}

bool Contains(const std::vector<std::string>& lines, const std::string& line) {
    return std::find(lines.begin(), lines.end(), line) != lines.end();
}

void AddBox(Engine::Registry& registry, float x) {
    const Engine::Entity box = registry.CreateEntity();
    registry.RequireComponent<Engine::Position>(box) = Engine::Position(x, 0.0f);
    Engine::ShapeComponent shape;
    shape.type = Engine::ShapeType::RECTANGLE;
    shape.width = 50.0f;
    shape.height = 50.0f;
    Engine::ColliderComponent collider;
    collider.shape = shape;
    registry.AddComponent(box, collider);
}
} // namespace

int main() {
    const fs::path project = fs::temp_directory_path() / "rtype_scene_systems_test";
    fs::remove_all(project);
    fs::create_directories(project);
    {
        // The menu script asks for the level on its first update. Its
        // onDestroy checks it receives both self and the entity.
        std::ofstream(project / "menu.lua") << R"(
local Script = {}
function Script:onCreate(entity)
    self.entity = entity
    log.info("[T] menu create " .. tostring(entity))
end
function Script:onUpdate(entity, dt)
    if not self.asked then
        self.asked = true
        Scene.load("level")
    end
end
function Script:onDestroy(entity)
    log.info("[T] menu destroy " .. tostring(entity) .. " self=" .. tostring(self.entity == entity))
end
return Script
)";
        std::ofstream(project / "level.lua") << R"(
local Script = {}
function Script:onCreate(entity)
    log.info("[T] level create " .. tostring(entity) .. " fresh=" .. tostring(self.entity == nil))
    self.entity = entity
end
function Script:onDestroy(entity)
    log.info("[T] level destroy " .. tostring(entity))
end
return Script
)";
    }

    Engine::Registry registry;
    registry.AddResource<Engine::ProjectContext>(project.string());
    registry.RegisterComponent<Engine::ShapeComponent>();
    Engine::ScriptPlugin().Init(registry);
    Engine::ColliderPlugin().Init(registry);
    auto& scenes = registry.AddResource<Engine::SceneManager>();

    scenes.Register("menu", [](Engine::Registry& r) {
        r.AddComponent(r.CreateEntity(), Engine::ScriptComponent("menu.lua"));
        AddBox(r, 0.0f); // two overlapping boxes -> one contact
        AddBox(r, 10.0f);
    });
    scenes.Register("level", [](Engine::Registry& r) {
        r.AddComponent(r.CreateEntity(), Engine::ScriptComponent("level.lua"));
        AddBox(r, 0.0f); // reuses the IDs of the menu boxes, but far apart
        AddBox(r, 500.0f);
    });

    std::size_t seen = 0;
    TakeScriptLines(seen);
    auto collisions = registry.GetSystem<Engine::CollisionSystem>();

    scenes.Load("menu");
    scenes.ApplyPending(registry);

    // Frame 1: menu script is created and asks for "level". The switch only
    // happens at the end of the frame, so collisions still ran on the menu.
    const auto order = registry.GetSystemUpdateOrder();
    const auto& systems = registry.GetSystems();
    for (const auto& id : order) {
        systems.at(id)->Update(registry, 1.0f / 60.0f);
    }
    CHECK(Contains(TakeScriptLines(seen), "menu create 1"));
    CHECK(collisions->GetActiveContacts().size() == 1);
    CHECK(scenes.HasPendingLoad());

    registry.FlushQueues();
    CHECK(scenes.ApplyPending(registry));
    CHECK(scenes.Current() == "level");

    // onDestroy ran for the menu script, with the right self and entity.
    const auto unloadLines = TakeScriptLines(seen);
    CHECK(Contains(unloadLines, "menu destroy 1 self=true"));
    // Old contact pair is gone even though IDs 2 and 3 exist again.
    CHECK(collisions->GetActiveContacts().empty());

    // Frame 2: the level script starts with a fresh table on reused ID 1,
    // and the far-apart boxes don't touch.
    RunFrame(registry);
    const auto levelLines = TakeScriptLines(seen);
    CHECK(Contains(levelLines, "level create 1 fresh=true"));
    CHECK(!Contains(levelLines, "menu create 1"));
    CHECK(collisions->GetActiveContacts().empty());

    // Switching again calls the level's onDestroy.
    scenes.Load("menu");
    RunFrame(registry);
    CHECK(Contains(TakeScriptLines(seen), "level destroy 1"));
    RunFrame(registry);
    CHECK(Contains(TakeScriptLines(seen), "menu create 1"));

    fs::remove_all(project);

    if (failures == 0) {
        std::cout << "SceneSystemsTest: all checks passed\n";
        return 0;
    }
    std::cerr << "SceneSystemsTest: " << failures << " check(s) failed\n";
    return 1;
}
