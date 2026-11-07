# Scene authoring

Cursor talks MCP to the editor CLI. The CLI loads and saves scenes through `SceneSerializer` and places UI with `UiCatalog`. The model never writes `.scene` JSON.

## Cursor

Build the editor, then point MCP at it (`Tools/AuthorMcp/mcp.json`):

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

Copy that file to `.cursor/mcp.json` if needed, and copy `Tools/Author/SKILL.md` to `.cursor/skills/rtype-author/SKILL.md`. Restart MCP and enable **rtype-author**.

`RTypeEditor mcp` and `RTypeEditor --mcp` are the same headless server. There is no window. See [Documentation/docs/editor/cli.md](../../Documentation/docs/editor/cli.md).

If the scene is open in the editor, reopen it after the tools run. Save in the editor would overwrite these writes.

## One-shot JSON

`rtype-author` still accepts one command object on stdin (handy for debugging). Prefer the CLI for Cursor.

```bash
echo '{"command":"list_ui_templates"}' | ./build/rtype-author
echo '{"command":"add_ui_screen","projectFolder":"Games/Menu","template":"menu","name":"MainMenu","startPage":true}' | ./build/rtype-author
```

## Test

```bash
cmake --build build --target McpCliTest
./build/McpCliTest
```
