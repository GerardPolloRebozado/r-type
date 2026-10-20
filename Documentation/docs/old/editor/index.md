# Editor

`RTypeEditor` is a native ImGui application that edits a live `Engine::Registry`. It is an
**authoring tool, not a runtime**: Lua scripts do not execute in the editor process, and the
multiplayer plugin is not loaded there. **Play** launches the real game binaries.

---

## Layout at a glance

```text
┌────────────────────────────┬───────────────────────────┐
│ Hierarchy                  │ Inspector                 │
│ entity tree, prefabs       │ selected entity components│
├────────────────────────────┼───────────────────────────┤
│ Scene View                 │ Assets / Preview          │
│ live game canvas           │ files and prefab editing  │
├────────────────────────────┴───────────────────────────┤
│ Console · Connections · Input Mapping · State · Settings│
└────────────────────────────────────────────────────────┘
```

Panels are dockable; the layout is saved in `imgui.ini`.

---

## Common workflows

<div class="grid cards" markdown>

-   **Create a project**

    ---

    Use the Gallery, or scaffold one from the terminal. A project contains `test.scene`,
    `assets/`, and `input.json`.

    [:octicons-arrow-right-24: Command line](cli.md)

-   **Build a scene**

    ---

    Spawn entities in Hierarchy, edit components in Inspector, drag assets into the Scene
    View.

    [:octicons-arrow-right-24: Interface](interface.md)

-   **Reuse a subtree**

    ---

    Copy/paste entities, drag them out as `.prefab`, and edit every instance through
    Preview.

    [:octicons-arrow-right-24: Interface → prefabs](interface.md#copy-paste-and-prefabs)

-   **Ship a scene**

    ---

    Save to `.scene`; the game deserializes the same file at startup.

    [:octicons-arrow-right-24: Scene serialization](serialization.md)

-   **Test with friends**

    ---

    Play Multiplayer launches one server and N clients on the current scene.

    [:octicons-arrow-right-24: Play multiplayer](multiplayer.md)

-   **Extend the inspector**

    ---

    Implement an `IPluginEditor` to expose a custom component.

    [:octicons-arrow-right-24: Editor plugins](plugins.md)

</div>

---

## Reference

| Page | Contents |
|---|---|
| [Interface](interface.md) | Every panel, the hierarchy, selection, and prefab flow |
| [Scene serialization](serialization.md) | `.scene` / `.prefab` format and reflection |
| [Editor plugins](plugins.md) | `IPluginEditor` and custom inspector widgets |
| [Command line](cli.md) | `new`, `create system`, `create plugin`, tests |
| [Play multiplayer](multiplayer.md) | Launch roles, arguments, recipes, troubleshooting |

---

## First time here?

Take the [editor tour](../start/editor-tour.md): Gallery → workspace → first moving entity →
prefab → Play.
