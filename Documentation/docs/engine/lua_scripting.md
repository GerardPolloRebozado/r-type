# Lua Scripting

The Lua Scripting subsystem provides a lightweight, hot-reloadable scripting layer for the R-Type Engine. It allows developers to write gameplay logic in Lua without recompiling the engine, using [sol2](https://github.com/ThePhD/sol2) (a C++17/23 binding library for Lua) and the system Lua 5.4 runtime.

This document is written **for engine developers** — people who will extend, maintain, or debug the scripting pipeline. It covers the full architecture, data flows, lifecycle contract, editor integration, build system, and common pitfalls.

---

## Table of Contents

1. [Architecture Overview](#architecture-overview)
2. [Dependency and Build Integration](#dependency-and-build-integration)
3. [Core Classes Deep-Dive](#core-classes-deep-dive)
4. [Script Lifecycle Contract](#script-lifecycle-contract)
5. [The Lua API Surface](#the-lua-api-surface)
6. [Editor Integration](#editor-integration)
7. [Serialization](#serialization)
8. [Build/Packaging Pipeline](#buildpackaging-pipeline)
9. [Extending the Lua API](#extending-the-lua-api)
10. [Pitfalls and Debugging Guide](#pitfalls-and-debugging-guide)

---

## Architecture Overview

The scripting pipeline mirrors the ECS plugin architecture. A Lua script is a **component** (`Engine::ScriptComponent`), attached to any entity. A dedicated **system** (`Engine::ScriptSystem`) owns the single shared `sol::state`, loads scripts on first encounter, and drives the Lua lifecycle hooks every frame.

```
┌──────────────────────────────────────────────────────────────────────┐
│                         Engine::Application                           │
│  ┌─────────────┐   ┌─────────────┐   ┌────────────────────────────┐ │
│  │  Registry   │───│ Component   │───│  ScriptComponent           │ │
│  │             │   │ Arrays      │   │   - scriptPath (string)    │ │
│  │  Resources  │   │             │   │   - autoReload (bool)      │ │
│  │  - AssetMgr │   │  Entity 8 ──│───│   - loaded (bool)          │ │
│  │  - Project  │   │   Script... │   │   - scriptTable (sol::table)│ │
│  │             │   └─────────────┘   └─────────────┬──────────────┘ │
│  └─────────────┘                                   │                │
│                        ┌────────────────────────────▼──────────┐   │
│                        │        Engine::ScriptSystem            │   │
│                        │  ┌──────────────────────────────────┐  │   │
│                        │  │  sol::state lua (shared, single) │  │   │
│                        │  │    ├─ base, table, string, math  │  │   │
│                        │  │    ├─ Entity.* / Transform.*     │  │   │
│                        │  │    ├─ Component.* / Input.*      │  │   │
│                        │  │    ├─ Time.* / log.* / print     │  │   │
│                        │  │    └─ KEY.* constants            │  │   │
│                        │  └──────────────────────────────────┘  │   │
│                        └────────────────────────────────────────┘   │
└──────────────────────────────────────────────────────────────────────┘
                                   │
                                   ▼
              ┌──────────────────────────────────────────┐
              │      scripts/<name>.lua  (file system)   │
              │   returns a table { onCreate, onUpdate } │
              └──────────────────────────────────────────┘
```

**Key design decisions:**

| Decision | Rationale |
|----------|-----------|
| One shared `sol::state` for all scripts | Scripts frequently reference each other (e.g. `08_simple_ai.lua` finding the "Player" entity). A shared global state lets module tables persist and share modules via Lua `require` if needed. |
| `ScriptComponent` stores the returned `sol::table` | Avoids re-parsing the Lua file every frame. Metadata `loaded` guards the lazy load. |
| Scripts **return a table** (`local M = {}; return M`) | Gives each script a private `self` scope and mirrors the standard Lua module convention. The system passes this table as `self` to lifecycle calls. |
| Path resolution belongs to the **project folder** | Both the Editor and the built game must find `scripts/*.lua`. The `ProjectContext` resource (Serialization) and the generated `main.cpp`'s `ChangeDirectory` cooperate to make this work. |

---

## Dependency and Build Integration

### 1. CPM Registration

sol2 is fetched via CPM in the root `CMakeLists.txt`:

```cmake
CPMAddPackage(
        NAME sol2
        GITHUB_REPOSITORY ThePhD/sol2
        VERSION 3.3.1
)
```

### 2. Engine Linking (the critical part)

`Engine/CMakeLists.txt` must expose sol2 and link system Lua 5.4 **in the correct order**:

```cmake
find_package(PkgConfig REQUIRED)
pkg_check_modules(LUA REQUIRED IMPORTED_TARGET lua5.4)

target_include_directories(Engine PUBLIC
        ${LUA_INCLUDE_DIRS}                       # -isystem .../lua5.4  (MUST be first)
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${RAYLIB_CPP_INCLUDE_DIRS}
        ${RAYLIB_CPP_INCLUDE_DIRS}/raylib-cpp
)
# sol2 is third-party: include as SYSTEM so its warnings don't trigger -Werror.
target_include_directories(Engine SYSTEM PUBLIC ${sol2_SOURCE_DIR}/include)
# sol2 3.3.1 + modern GCC emits -Wtemplate-body warnings; silence them.
target_compile_options(Engine PUBLIC -Wno-template-body -Wno-error=template-body)
target_link_libraries(Engine PUBLIC PkgConfig::LUA)
```

> ⚠️ **Why `lua5.4` and not `lua`?** Many distributions ship multiple Lua version headers in `/usr/include`. The default `lua` pkg-config may point at **Lua 5.5**, whose `lua_newstate` gained an extra `seed` parameter that sol2 3.3.1 cannot parse — producing cryptic `duplicate case value` and `too few arguments to lua_newstate` errors. Always force the Lua 5.4 pkg-config variant.

The `SYSTEM` include + `-Wno-template-body` are mandatory for GCC 16+, which treats sol2's internal `optional_implementation.hpp` warnings as errors.

### 3. New Source Files

These files are tracked in `Engine/CMakeLists.txt` (explicit `SRC_FILES` list — **not glob**, so add them manually):

```
src/Scripting/ScriptPlugin.cpp
src/Scripting/ScriptSystem.cpp
src/Scripting/LuaAPI.cpp
```

Headers:

```
include/Scripting/ScriptComponent.hpp
include/Scripting/ScriptPlugin.hpp
include/Scripting/ScriptSystem.hpp
include/Scripting/LuaAPI.hpp
```

---

## Core Classes Deep-Dive

### `Engine::ScriptComponent` (`include/Scripting/ScriptComponent.hpp`)

A plain data component. **It has no Lua dependencies in its public interface** other than storing the cached `sol::table`. This keeps it trivially serializable and cheap to copy.

```cpp
struct ScriptComponent {
    std::string scriptPath;          // project-relative, e.g. "scripts/player.lua"
    bool        autoReload = true;   // watched by future hot-reload system
    bool        loaded    = false;   // guards lazy one-time load
    sol::table  scriptTable = sol::lua_nil;  // cached returned module table
};
```

**Why store `sol::table` and not a raw pointer?** sol2's `sol::table` is a lightweight reference wrapper (holds a `lua_State*` + registry index). It is safe to copy and survives across frames as long as the owning `ScriptSystem.lua` state lives. The system always outlives the registry, so lifetime is guaranteed.

### `Engine::ScriptPlugin` (`include/Scripting/ScriptPlugin.hpp`)

Standard ECS plugin. Its job is only registration:

```cpp
void ScriptPlugin::Init(Registry& registry) {
    registry.RegisterComponent<ScriptComponent>();
    registry.RegisterSystem<ScriptSystem>();
}
```

It must be installed **after** every other plugin whose components scripts will bind (Position, Sprite, Shape, Text, Collider, Rotation). See the boot sequence in [Editor Integration](#editor-integration).

### `Engine::ScriptSystem` (`include/Scripting/ScriptSystem.hpp`)

Owns the single `sol::state lua`. Registers the C++ API into it lazily on the first `Update`.

```cpp
class ScriptSystem : public System {
  public:
    ScriptSystem();
    ~ScriptSystem();
    void Update(Registry& registry, float dt) override;
    void Render(Registry& registry) override {}  // scripts have no render phase

  private:
    sol::state lua;         // shared across all scripted entities
    bool apiRegistered = false;
    void loadScript(Registry& registry, Entity entity);
    void unloadScript(Registry& registry, Entity entity);

    static std::unordered_set<std::string> s_warnedMissingScripts;
};
```

**Update loop** (simplified):

```cpp
void ScriptSystem::Update(Registry& registry, float dt) {
    if (!apiRegistered) {                       // one-time boot
        lua.open_libraries();
        LuaAPI::registerEngineAPI(lua.lua_state(), registry);
        apiRegistered = true;
    }
    for (size_t i = 0; i < registry.GetEntityCount(); ++i) {
        Entity entity{i};
        if (!registry.HasComponent<ScriptComponent>(entity)) continue;
        auto& script = registry.GetComponent<ScriptComponent>(entity);

        if (!script.loaded && !script.scriptPath.empty())     // lazy load
            loadScript(registry, entity);

        if (script.loaded && script.scriptTable.valid()) {
            sol::optional<sol::function> onUpdate = script.scriptTable["onUpdate"];
            if (onUpdate && onUpdate->valid()) {
                // self injected so `function M:onUpdate()` (colon syntax) works
                auto result = onUpdate->call(script.scriptTable, entity, dt);
                if (!result.valid()) { log_error(...); }
            }
        }
    }
}
```

**Purpose of `s_warnedMissingScripts`:** In the Editor, the process working directory is not the project folder, so a relative `scripts/foo.lua` fails to resolve and would spam `[ERROR] Script not found` **every frame**. The static set remembers the path we've already complained about and downgrades to a single warning.

---

## Script Lifecycle Contract

A Lua script **must return a table** whose keys are optional lifecycle functions:

```lua
local M = {}

function M:onCreate(entity) end   -- called once, after the file is loaded
function M:onUpdate(entity, dt) end  -- called every frame
function M:onDestroy(entity) end  -- called when the component is unloaded

return M
```

| Hook | Arguments | When it runs | Notes |
|------|-----------|--------------|-------|
| `onCreate(entity)` | `entity` (integer id) | Exactly once, right after `lua.script_file()` returns a valid table. | Use it to initialize `self`, add components, register state. |
| `onUpdate(entity, dt)` | `entity`, `dt` (seconds) | Every frame while the entity has a loaded ScriptComponent. | `dt` is the system timestep passed from `ScriptSystem::Update`. |
| `onDestroy(entity)` | `entity` | When `unloadScript` runs (currently on hot-reload / removal). | Cleanup coroutines, remove child entities. |

> **Colon vs dot:** Because the system calls `onUpdate->call(scriptTable, entity, dt)`, the script table is bound to `self`. Scripts therefore use the **method** syntax `function M:onUpdate(entity, dt)`. A lone `function M.onUpdate(dt)` would receive `self` implicitly as the first declared parameter and shift all arguments — always use colon syntax.

Errors are caught: if a hook returns a `sol::error`, the message is logged with the entity id and execution continues. A failing script never interrupts the game loop, but the failed frame's work is lost.

---

## The Lua API Surface

All C++ functions are registered by `Engine::LuaAPI::registerEngineAPI(lua_State* L, Registry& registry)` in `src/Scripting/LuaAPI.cpp`. The `L` pointer is captured **by value** (not `sol::state_view` by reference) in every lambda, because the local `state_view` is destroyed when `registerEngineAPI` returns — capturing it by reference is a classic dangling-reference segfault (see [Pitfalls](#pitfalls-and-debugging-guide)).

### Globals set up by `openStandardLibraries`

```
base, table, string, math, debug, io, os, package
```

### `Entity.*`

| Function | C++ binding | Description |
|----------|-------------|-------------|
| `Entity.create()` → `integer` | `registry.CreateEntity()` | Creates a new entity and returns its id. |
| `Entity.destroy(entity)` | `registry.DestroyEntity(e)` | Queues entity for destruction. |
| `Entity.find(name)` → `integer\|nil` | linear scan of `Name` components | Returns the first matching entity, or nil. O(n) — fine for debug, avoid in hot loops for large scenes. |
| `Entity.getName(entity)` → `string` | reads `Name` | Empty string if no Name component. |
| `Entity.setName(entity, name)` | upserts `Name` | |

### `Transform.*`

| Function | Signature | Description |
|----------|-----------|-------------|
| `getPosition(entity)` | → `{x, y}` table | Returns position; creates a fresh table each call. |
| `setPosition(entity, x, y)` | | Sets or creates `Position`. |
| `translate(entity, dx, dy)` | | Adds a delta. |
| `getRotation(entity)` → `float` | | Degrees; 0 if no `Rotation`. |
| `setRotation(entity, deg)` | | |
| `rotate(entity, delta)` | | Adds a delta. |

> **Return by table:** `getPosition` returns a plain `{x=…, y=…}` Lua table, not the C++ `Position` usertype. This avoids sol2 usertype copy-construction friction with `Position`'s multi-field layout (prevX/prevY/moved/relative).

### `Component.*`

| Function | Signature | Effect |
|----------|-----------|--------|
| `addSprite(entity, path, ox, oy, sx, sy)` | strings/floats | Loads via `AssetManager`, adds `Position`+`Rotation` if missing, adds `SpriteComponent`. |
| `addShape(entity, type, w, h, r, sides, thick, filled, cr, cg, cb, ca)` | ints/floats/bool | Adds `ShapeComponent`. `type` maps to `ShapeType` enum. |
| `addText(entity, text, size, cr, cg, cb, ca)` | | Adds `Text`. |
| `addBoxCollider(entity, w, h, isTrigger)` | | Adds `ColliderComponent` with a rectangle shape. |
| `has(entity, "Sprite")` | string tag | Compile-time-free tag dispatch. Valid tags: `Position, Rotation, Sprite, Shape, Text, Name, Collider, Parent`. |
| `remove(entity, "Shape")` | tag | Dispatch to `registry.RemoveComponent<T>`. |

### `Input.*` / `KEY.*`

Directly bound to Raylib polling (simplest and lowest-latency option for scripted prototypes):

```
Input.isKeyDown(key) / Input.isKeyPressed(key) / Input.isKeyReleased(key)
Input.getMousePosition() → {x, y}
```

`KEY.W`, `KEY.SPACE`, `KEY.LEFT_SHIFT`, … are plain integer constants mirroring Raylib key codes.

### `Time.*` and logging

```
Time.deltaTime()  -- placeholder constant; prefer the dt passed to onUpdate
Time.time()       -- seconds since start (raylib GetTime)
log.info(msg) / log.error(msg) / log.debug(msg)
print(...)        -- variadic, joins args into a single log.info line
```

`log.*` push through `Engine::log_*`, which both prints to stdout/stderr and stores in `LogBuffer` — that is how lines appear in the Editor's Console Panel.

---

## Editor Integration

### Boot sequence (`Editor/main.cpp`)

The Editor must register `ScriptPlugin` **after** the component plugins, before opening a scene:

```cpp
Engine::PositionPlugin  positionPlugin;  positionPlugin.Init(app.GetRegistry());
Engine::TextPlugin      text_plugin;     text_plugin.Init(app.GetRegistry());
Engine::SpritePlugin    spritePlugin;    spritePlugin.Init(app.GetRegistry());
Engine::ShapePlugin     shapePlugin;     shapePlugin.Init(app.GetRegistry());
Engine::RotationPlugin  rotationPlugin;  rotationPlugin.Init(app.GetRegistry());
Engine::ColliderPlugin  colliderPlugin;  colliderPlugin.Init(app.GetRegistry());
Engine::ScriptPlugin    scriptPlugin;    scriptPlugin.Init(app.GetRegistry());
editorLayer.SetRegistry(app.GetRegistry());
editorLayer.SetConfiguration(config);
editorLayer.Init();   // deserializes the scene, which may create ScriptComponents
```

Because `ScreenRenderer` (the Editor viewport) renders via `customRender`, the editor runs the same `ScriptSystem::Update` through `Application::Update()` — so scripts behave identically in the preview as in the built game, modulo working directory.

### Inspector plugin (`Editor/src/Plugins/ScriptPluginEditor.cpp`)

Implements the `Editor::IPluginEditor` contract and is registered in `InspectorPanel::Init()`:

```cpp
m_PluginEditors.push_back(std::make_unique<ScriptPluginEditor>());
```

It provides:

- A **Script Path** `InputText` field (projects the path into `script.scriptPath`).
- A **Drag & Drop** target for `"CONTENT_BROWSER_ITEM"` payloads from the Assets panel.
- An **Auto Reload** checkbox.
- A **Create New Script** button that opens a modal, asks for a name, writes `<proj>/scripts/<name>.lua` from a template, and sets the path.
- A colored status line: **Loaded** (green) / **Not loaded yet** (amber) / **File not found** (red).

### Project-context plumbing

`EditorLayer::SetConfiguration` injects two resources used by the Editor and serializer:

```cpp
m_Registry->AddResource<ProjectContext>(config.m_ProjectFolder);      // Serialization namespace
m_Registry->AddResource<EditorConfig>(config);                        // for the Inspector modal
```

The `ProjectContext` is what `ScriptSystem::loadScript` consults to turn `scripts/foo.lua` into an absolute path inside the Editor preview.

---

## Serialization

`ScriptPluginSerializer` (`Serialization/src/Plugins/ScriptPluginSerializer.cpp`) is a standard `IPluginSerializer`:

```cpp
outEntityJson[GetKey()] = {
    {"scriptPath", script.scriptPath},
    {"autoReload", script.autoReload},
};
```

On load it constructs a default `ScriptComponent` (only `scriptPath`/`autoReload` — the heavy `scriptTable` starts as `sol::lua_nil` and is lazily populated on first `onCreate`).

It is registered in `ComponentSerializer::EnsureInitialized()`:

```cpp
s_Serializers.push_back(std::make_unique<ScriptPluginSerializer>());
```

The JSON fragment embedded in a scene looks like:

```json
"ScriptComponent": {
    "scriptPath": "scripts/player.lua",
    "autoReload": true
}
```

---

## Build/Packaging Pipeline

When the user presses **Build**, `Editor/src/Panels/SettingsPanel.cpp::StartBuild` regenerates `<proj>/main.cpp` and links Engine+Serialization via a temporary `.editor_build.cmake` bridge. The generated `main()`:

1. Calls `ChangeDirectory(GetApplicationDirectory())` → sets the process cwd to the executable's folder.
2. Initializes all plugins, including `ScriptPlugin`.
3. Deserializes every `*.scene` in the project folder.

Because the CMake `POST_BUILD` step copies the executable next to `test.scene` and the `scripts/` folder already lives in the project folder, the game's working directory matches the project folder and relative script paths resolve.

> ⚠️ **If you ever change the publishing layout** so the exe is *not* copied to the project root, script loading (and any relative asset path) will break. Either keep the copy step, or start propagating absolute paths from `ProjectContext`.

---

## Extending the Lua API

To expose a new engine type or function:

1. Add a new `lua["Module"]["fn"] = [&registry](args){…};` entry inside `registerEngineAPI`.
2. **Capture `rawL` (the `lua_State*`) by value if you need to create tables or return sol objects.**
3. Prefer returning primitives or plain `{x,y}` tables over C++ usertypes when possible.
4. Rebuild `Engine`; nothing else changes automatically since the API is built at runtime inside `sol::state`.

Example: adding a `Scene.timeScale`:

```cpp
lua["Scene"]["setTimeScale"] = [&registry](float ts) {
    if (registry.HasResource<TimeScale>())
        registry.GetResource<TimeScale>().value = ts;
};
```

---

## Pitfalls and Debugging Guide

| Symptom | Cause | Fix |
|---------|-------|-----|
| `duplicate case value call_status` at compile | System Lua 5.5 headers picked up by sol2 | Use `pkg_check_modules(LUA lua5.4)` and put `${LUA_INCLUDE_DIRS}` **first**. |
| `error: 'class sol::optional<T&>' has no member 'construct'` | GCC 16 + sol2 3.3.1 warning promoted to error | Add `-Wno-template-body -Wno-error=template-body` and include sol2 as `SYSTEM`. |
| `undefined reference to vtable for ScriptPluginSerializer` | CMake glob didn't pick up a new `.cpp` | `touch` the CMakeLists to re-glob, or add the file to the explicit source list. |
| `undefined reference to vtable for Editor::ScriptPluginEditor` | Editor uses explicit `SRC_FILES`, not glob | Add the new file to `Editor/CMakeLists.txt`. |
| Segfault in `lua_createtable` from an `Engine::LuaAPI` lambda | Lambda captured `&lua` (local `sol::state_view`) by reference; dangling after `registerEngineAPI` returns | Capture `lua_State* rawL` by value; construct `sol::state_view view(rawL)` inside the lambda. |
| `attempt to index a number value (local 'self')` in `onCreate` | Called `create->call(entity)` without passing the module table as `self` | Call `create->call(script.scriptTable, entity)`. |
| `[ERROR] Script not found` spamming the Editor Console | Editor process cwd != project folder, and each frame retries load | `s_warnedMissingScripts` set logs once; guaranteed resolution relies on `ProjectContext` being registered before preview. |
| `bad argument into 'Engine::Position(long unsigned int)'` | Returning the `Position` usertype by value from a binding | Return a `{x,y}` table built from a fresh `sol::state_view`. |
| Scripts load in game but not in Editor preview | `ScriptPlugin` not registered in `Editor/main.cpp`, or registered *after* scene deserialize | Register before `editorLayer.Init()`. |

### Debug checklist

1. **Compile-time Lua version:** check `pkg-config --cflags lua5.4` returns `-I/usr/include/lua5.4`, and that it appears before other `-I` paths in the compile log.
2. **Runtime load failure:** temporarily add `log_info("cwd=" + std::filesystem::current_path().string())` in `ScriptSystem::Update`.
3. **Hook not firing:** verify the script `return M` is at the bottom of the file — a script that returns nothing logs `Script must return a table`.
4. **Sol2 crash inside a callback:** confirm no lambda captures a local `sol::state_view`/`sol::state` by reference.

---

## Future Work (roadmap)

- **True hot-reload:** use a file-watcher plus `sol::table` invalidation to reload `.lua` on save without restarting. The `autoReload` field and `unloadScript` plumbing are already in place.
- **Component getters that return live references** so scripts can mutate Sprite scale / Text content instead of destroying and re-adding.
- **Collision events:** expose `onCollision(other, contactPoint)` by plumbing `CollisionEvents` into the ScriptSystem and dispatching per entity script.
- **Coroutines & helpers:** a frame-accurate `coroutine` wrapper for timer and animation callbacks.
- **Sandboxing:** disable `io`/`os` in `openStandardLibraries` for shipping builds to prevent arbitrary file access from untrusted scripts.
