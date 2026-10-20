# Editor tour

The editor is a native ImGui application that edits a live `Registry`. There is no import
step: the scene you see is the data the game will run.

This tour covers the layout, the core workflow, and the shortcuts worth memorising. For the
full reference, see [Editor → Interface](../editor/interface.md).

---

## 1. The Gallery

Launch the editor with no arguments:

```bash
./build/RTypeEditor
```

The **Gallery** lists your projects. From here you can:

- create a project with `./RTypeEditor new <name> [--path <dir>]`;
- reopen an existing project by selecting it;
- open a scene directly: `./build/RTypeEditor path/to/test.scene`.

A project is a folder with a `test.scene`, an `assets/` directory, and an `input.json`. See
[Command line](../editor/cli.md) for the exact layout.

---

## 2. The workspace

```text
┌─────────────────────────────┬──────────────────────────┐
│ Hierarchy                   │ Inspector                │
│ entities, prefabs           │ components of selection  │
├─────────────────────────────┼──────────────────────────┤
│ Scene View                  │ Assets / Preview         │
│ the game canvas             │ files, prefab preview    │
├─────────────────────────────┴──────────────────────────┤
│ Console · Connections · Input Mapping · State · Settings│
└────────────────────────────────────────────────────────┘
```

| Panel | What it does |
|---|---|
| **Hierarchy** | Lists entities and their parent/child tree; right-click to spawn |
| **Inspector** | Edits the components of the selected entity in place |
| **Scene View** | Renders the world through the engine's camera at virtual resolution |
| **Assets** | Browses the project's `assets/` folder; drag files into the scene |
| **Preview** | Shows the selected prefab and edits its shared data |
| **Connections** | Inspects signal links between entities |
| **Console** | Engine logs and errors |
| **Input Mapping** | Edits `input.json` actions and keybinds |
| **State** | Creates and previews state resources |
| **Settings** | Project, build, and multiplayer settings |

Panels are dockable. Drag a tab to rearrange, or close panels you do not use. The layout is
stored in `imgui.ini` next to the executable.

---

## 3. Make something move

1. In **Hierarchy**, right-click the empty area and spawn an entity.
2. Select it. In **Inspector**, open **Position** and set `x`, `y`, and `z`.
3. Add a **Shape** or **Sprite** component: right-click in **Inspector** and pick it from
   the component menu.
4. If you used a sprite, select an image in **Assets** and assign it.
5. Drag the entity in the **Scene View** to place it; the Inspector values update live.

Everything you change is written straight into the `Registry`. Save with ++ctrl+s++ to write
the `.scene` file.

---

## 4. Prefabs and copy/paste

- ++ctrl+c++ / ++ctrl+v++ copies and pastes the selected subtree.
- Right-click a row for the same actions, plus **Paste**.
- Drag a Hierarchy row onto **Assets** to write a `.prefab` in the current folder.
- Drag a `.prefab` back onto the Hierarchy background (or a row) to instantiate it.
- Select a `.prefab` in Assets to edit all instances through **Preview**.

A pasted entity keeps its parent and position, so it overlaps the original until you move it.
Names are deduplicated (`enemy1`, `enemy1(1)`, …).

---

## 5. Run the game

The editor is an authoring tool: Lua does not run inside it, and the multiplayer plugin is
not loaded into the editor process. **Play** and **Play Multiplayer** launch the real game
binaries with the current scene.

- **Play** starts one game process.
- **Play Multiplayer** starts one server and N clients, each in its own window; **Stop**
  closes them all.

The first Play may compile the project. Build output appears in the Console panel.

---

## 6. Keyboard rhythm

| Shortcut | Action |
|---|---|
| ++ctrl+s++ | Save the scene |
| ++ctrl+c++ / ++ctrl+v++ | Copy / paste the selected subtree (Hierarchy focused) |
| ++delete++ | Delete the selected entity (Hierarchy focused) |

Shortcuts are processed only while the relevant panel has focus and no text field is active.
Click the Hierarchy first if a key seems to do nothing.

---

## Next

- [Interface reference](../editor/interface.md) — every panel in detail.
- [Scene serialization](../editor/serialization.md) — what a `.scene` contains.
- [Command line](../editor/cli.md) — scaffold projects, systems, and plugins.
- [Play multiplayer](../editor/multiplayer.md) — the full launch workflow.
