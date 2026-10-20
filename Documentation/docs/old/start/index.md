# Start here

New to the engine? Follow this path once, in order. It gets you from a clone to a moving
scene and then into the editor.

<div class="grid cards" markdown>

-   **1 · Install**

    ---

    Clone, install the system libraries, configure with CMake, and build the editor and
    games.

    [:octicons-arrow-right-24: Installation](installation.md)

-   **2 · Build a game**

    ---

    Create an `Application`, register a plugin, spawn an entity, and write your first
    system. About ten minutes.

    [:octicons-arrow-right-24: Your first game](first-game.md)

-   **3 · Open the editor**

    ---

    Tour the Gallery, the panels, the prefab workflow, and the Play buttons.

    [:octicons-arrow-right-24: Editor tour](editor-tour.md)

-   **4 · Understand the model**

    ---

    Entities, components, systems, resources, and why the registry is the centre of
    everything.

    [:octicons-arrow-right-24: ECS architecture](../engine/ecs.md)

</div>

---

## What you are building

The engine is a C++23 ECS runtime with a data-driven editor on top:

- **One registry** owns entities, components, systems, and resources.
- **Everything is a plugin.** Audio, sprites, collisions, networking, and scripting are
  opt-in bundles.
- **The editor and the game read the same scene.** The editor edits a live registry and
  serializes it to `.scene`; the game deserializes it and runs.
- **Multiplayer is server-authoritative.** Tag an entity with `NetworkComponent`; the
  engine handles identity, spawning, input, and state sync.

---

## Where to go next

| Goal | Page |
|---|---|
| Build a menu or a HUD | [Text](../engine/text.md) · [States](../engine/states.md) |
| Load images and sound | [Assets](../engine/assets.md) · [Audio](../engine/audio.md) |
| Move the camera, add screen shake | [Camera](../engine/camera.md) |
| Detect hits and triggers | [Collisions](../engine/collisions.md) |
| Add local multiplayer input | [Input](../engine/input.md) |
| Add networked play | [Networking](../engine/networking.md) · [Play multiplayer](../editor/multiplayer.md) |
| Scaffold code from the terminal | [Command line](../editor/cli.md) |

!!! tip "Looking for a class or function?"
    The [API reference](../api/index.md) is generated from the engine headers by Doxygen.
    Use the search bar (++slash++) when you already know the symbol name.
