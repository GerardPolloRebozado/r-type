# What's included

A quick inventory before you dive in. Everything listed here ships in the repository; the
linked page explains how to use it.

---

## Core

| Feature | Notes |
|---|---|
| **ECS registry** | Entities, contiguous component arrays, ordered systems, global resources |
| **Plugin system** | One `Init()` per feature; registration order is update order |
| **Application loop** | Fixed-step update, variable render, virtual resolution, headless mode |
| **Signals** | Typed events between systems and scripts |
| **Logging** | Buffered logger with levels, consumed by the editor Console |
| **Lua scripting** | `ScriptComponent` + sol2 bindings for gameplay scripts |
| **Scene serialization** | JSON `.scene` and `.prefab` files, reflection-driven |
| **Asset bundling** | `AssetBundler` compiles `assets/` into the executable |

## Rendering

| Feature | Notes |
|---|---|
| **Sprites** | Offsets, scale, flip, layered by `Position::z` |
| **Shapes** | Rectangles, circles, regular polygons, lines |
| **Text** | Bitmap and TTF/OTF fonts, spacing, colour, screen or world space |
| **Camera** | Follow, zoom, rotation, bounds, screen shake |
| **Virtual resolution** | Fixed canvas letterboxed into resizable windows |
| **Screen-space pass** | HUD entities ignore the camera |

## Gameplay

| Feature | Notes |
|---|---|
| **Input** | Keyboard, mouse, gamepad; string actions via `input.json` |
| **Collisions** | SAT for rectangles, circles, polygons; layers and trigger events |
| **Audio** | ECS-driven sounds and streamed music, overlapping voices |
| **Hierarchy** | Parent/child transforms and subtree destruction |
| **Prefabs** | Spawn, edit, and share entity subtrees |
| **States** | `State<T>` / `NextState<T>` resources with transition events |

## Networking

| Feature | Notes |
|---|---|
| **Server-authoritative replication** | Spawn, state, and despawn snapshots |
| **Role selection** | Single-player, server, or client at startup |
| **Input replication** | Clients send input; the server simulates |
| **RPCs** | Declarative packet handlers on top of ENet |
| **Low-level SDK** | Direct `Net.send` / `Net.on` when you need it |

## Editor

| Feature | Notes |
|---|---|
| **Gallery** | Project list, creation, and recent scenes |
| **Live inspector** | Every registered component, edited in place |
| **Scene view** | The real game camera at virtual resolution |
| **Assets** | Browse, drag into the scene, preview prefabs |
| **Play / Play Multiplayer** | Launch one game or a server plus N clients |
| **Scaffolding CLI** | Create projects, systems, and plugins from the terminal |

---

## Deliberate non-goals

- **3D.** The renderer and collision pipeline are 2D.
- **A visual shader graph.** Shaders are raylib shaders, written in code.
- **A runtime scene editor inside the game.** The editor is a separate process.
- **Automatic ECS signatures.** Systems declare what they iterate; the registry does not
  scan components to fill `m_Entities`.

---

Ready? Start with [Installation](installation.md), or read [ECS architecture](../engine/ecs.md)
first if you prefer the theory before the code.
