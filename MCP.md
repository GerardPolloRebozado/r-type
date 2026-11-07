# MCP: let the AI edit a scene

The AI in Cursor can change a game scene for you. It does not type the scene file by hand. It calls small tools. Those tools use the same code as the editor (create an entity, place a UI template, save the scene).

The server is the editor itself, started with no window:

```bash
./build/RTypeEditor mcp
```

`--mcp` does the same thing.

## What was added

| Piece | What it is |
|---|---|
| `RTypeEditor mcp` | The server Cursor talks to. No Gallery, no window. |
| `Tools/Author/` | The commands: create entity, place UI, write a Lua script, save the scene. |
| `Tests/McpCliTest.cpp` | Checks that a tool call really changes a `.scene` file. |
| `.cursor/mcp.json` | Tells Cursor to start `RTypeEditor mcp`. |

`Tools/AuthorMcp/server.py` is retired. Do not point Cursor at that Python file.

## How it fits together

```text
You, in Cursor chat
        |
        v
Cursor's AI  --calls a tool-->  RTypeEditor mcp
                                      |
                                      v
                              loads the .scene
                              changes the registry
                              saves the .scene
```

Example: you say "add a Quit button". The AI calls `add_ui_screen` and `add_ui_widget`. The server creates the entities, parents, and page names. You reopen the scene and see them in Hierarchy.

The open editor window is not updated live. The server writes the file on disk.

## Turn it on

1. Build the editor (once, and again after you pull):

```bash
cmake --build build --target RTypeEditor
```

2. Cursor already has `.cursor/mcp.json`. If it is missing, copy `Tools/AuthorMcp/mcp.json` to `.cursor/mcp.json`:

```json
{
  "mcpServers": {
    "rtype-author": {
      "command": "${workspaceFolder}/build/RTypeEditor",
      "args": ["mcp"],
      "env": {
        "RTYPE_ENGINE_ROOT": "${workspaceFolder}"
      }
    }
  }
}
```

On Windows the binary may be `build/Debug/RTypeEditor.exe` or `build/Release/RTypeEditor.exe`. Put that path in `command`.

3. Restart MCP in Cursor and enable **rtype-author**.

4. In chat, name the game folder. Then ask for the change.

## Ask for a menu

> In Games/Menu, add a main menu with a Quit button.

The AI should do roughly this:

1. `add_ui_screen` with template `menu` and name `MainMenu`.
2. `add_ui_widget` with screen `mainmenu`, template `button`, label `Quit`, and click action `quit`.

You can do the same from the terminal, one command at a time (`rtype-author` is the debug tool; Cursor uses `RTypeEditor mcp`):

```bash
echo '{"command":"add_ui_screen","projectFolder":"Games/Menu","template":"menu","name":"MainMenu","startPage":true}' | ./build/rtype-author
echo '{"command":"add_ui_widget","projectFolder":"Games/Menu","screen":"mainmenu","template":"button","props":{"label":"Quit"},"onClick":{"type":"quit"}}' | ./build/rtype-author
```

Then in the editor: close the scene if it is open, and open it again. You should see MainMenu and a Quit button.

## Ask for an enemy

> In R-Type, create an entity named Drifter at x 400, y 0, with sprite assets/ship.png, and a script that moves left.

The AI should:

1. `create_entity` with name `Drifter`, position, and a sprite component.
2. `attach_script` with only the `onUpdate` body, for example `Transform.translate(entity, -80 * dt, 0, 0)`.

The tool writes the rest of the Lua file (`local Script = {}`, `onCreate`, `return Script`) and attaches it. You do not write that boilerplate, and neither does the AI.

## Other things you can ask

- "What UI templates exist?" → `list_ui_templates`
- "What is in this scene?" → `scene_outline` (names and components, not the raw file)
- "Place Prefabs/Pata-Pata.prefab at x 100, y -50" → `place_prefab`
- "Add a wave of 5 of that prefab" → `add_wave`
- "How does Entity.create work?" → `lua_api` with topic `entity` (one doc page, not the whole manual)

Click actions on a button are words the tool already knows: `none`, `goToPage`, `back`, `callMethod`, `loadScene`, `quit`. A "go to settings" button looks like `"onClick": {"type": "goToPage", "target": "settings"}`.

## After the AI finishes

1. If the scene is open in the editor, **reopen it**. Do not press Save first. Save writes the old copy from memory and deletes the AI's change.
2. Press Play and check the result.

Script files that are already open in the script editor reload from disk on their own.

## Check that it works

```bash
cmake --build build --target McpCliTest
./build/McpCliTest
```

You want the last line to be `McpCliTest: all checks passed`. That test starts `RTypeEditor mcp`, adds a menu, and checks the scene file.

## What the AI is not allowed to do

It should not edit a `.scene` or `.prefab` file as text. Those files are a snapshot of the registry (entity ids, parents, optional fields). The tools own that. If a template or component changes later, the tools pick it up. Old prompts do not have to learn a new file format.

This runs only on your machine. The tools can write scenes and scripts inside the project you point them at.
