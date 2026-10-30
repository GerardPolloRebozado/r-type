# Interface

The editor is a dockable ImGui workspace. Every panel reads and writes the same
`Engine::Registry`; there is no separate editor scene format until you save.

## Panels

| Panel | Purpose |
|---|---|
| **Hierarchy** | Entity tree, selection, copy/paste, prefab drops |
| **Inspector** | Component editors for the selected entity |
| **Scene** | The game rendered at virtual resolution through the engine camera |
| **Assets** | Project asset browser; drag images and prefabs into the scene |
| **Preview** | Selected image or prefab preview; edits shared prefab data |
| **Connections** | Signal subscriptions between entities |
| **Console** | Engine logs, errors, and build output |
| **Input Map** | Actions and keybinds stored in `input.json` |
| **State** | State resources, initial state, and live activation |
| **Settings** | Project settings, build, and multiplayer launch |
| **Script Editor** | Lua source editing with syntax highlighting |

Drag tabs to rearrange. The dock layout is stored in `imgui.ini` next to the executable.

## Scene view

The scene view is not an approximation: it renders the registry with the same systems and
camera the game uses, at the project's virtual resolution. Resize the window and the canvas
letterboxes exactly as it will in the game.

- Click an entity to select it.
- Drag the centre handle to move it; drag the X, Y, or rotation handles to constrain.
- Right- or middle-drag to pan the editor camera.
- Scroll to zoom the editor view (this does not change the game camera).

Runtime-only behaviour (Lua, multiplayer) is not simulated here; use **Play**.

## Hierarchy

The tree shows parent/child relationships. Entities without a `Parent` are roots.

- **Right-click empty space** to spawn an entity.
- **Right-click a row** for copy, paste, rename, delete, and prefab actions.
- **Drag a row onto another row** to parent it; drop on empty space to detach.
- **Delete** removes the selected entity and its descendants.

A `Name` component is what the editor displays. Entities without one fall back to their id.

## Inspector

The inspector shows one collapsible section per component on the selected entity.

- **Drag a value** to change it; changes apply immediately.
- **Right-click empty space** to add a component from the registered list.
- **Right-click a component header** to remove or reset it.
- Components without a dedicated editor plugin still appear with a generic field editor.

Edits are written directly into component memory. Save before large changes.

## Assets

The assets panel browses the project's `assets/` directory.

- Double-click an image to preview it.
- Drag an image onto an entity in the Hierarchy or Scene view to assign a sprite.
- Drag a Hierarchy row onto the panel to write a `.prefab`.
- Drag a `.prefab` back to instantiate it.

The path stored in components is project-relative, so the same scene works from the editor
and from the built game.

## Copy, paste, and prefabs

Right-click a Hierarchy row, or press ++ctrl+c++ while the Hierarchy is focused, to copy an
entity and its children. ++ctrl+v++ creates a new subtree with the same data.

A pasted entity keeps the original parent and position, so it overlaps the original until
you move it. Names are deduplicated: `enemy1` becomes `enemy1(1)`, then `enemy1(2)`. The
same numbering applies to instantiated prefabs.

Dragging a row to **Assets** writes `<Name>.prefab` in the folder you are viewing. If the
name exists, the next file is `Name_2.prefab`, then `Name_3.prefab`.

Selecting a `.prefab` in Assets opens it in **Preview**. Changing the sprite path, scale,
shape size, or script there updates every instance. Positions stay where each instance was
placed.

## Console

Engine logs are buffered and shown here with severity colouring. Build output from **Play**
also lands in the Console, which is the first place to look when a launch fails.

## Settings and Play

Project settings cover the window, virtual resolution, target FPS, and build target. The
Play buttons launch the real binaries:

- **Build** (hammer) compiles the project.
- **Play**  -  one game process with the current scene.
- **Play Multiplayer**  -  one server plus N clients, each in its own window.
- **Stop**  -  closes every process started by the current Play session.

See [Play mode](play.md) and [Multiplayer](multiplayer.md).

## See also

- [Scenes](scenes.md)
- [Prefabs](prefabs.md)
- [Entities](entities.md)
- [Editor plugins](../plugins/editor-plugins.md)
- [Command line](cli.md)
