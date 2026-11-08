---
name: rtype-author
description: >-
  Authors Lucky & Pozzo / R-Type editor games through rtype-author MCP tools
  (add_ui_screen, add_ui_widget, create_entity, attach_script, place_prefab).
  Use when vibecoding a game, placing UI, creating entities, writing Lua
  gameplay hooks, or editing a .scene in this repo. Do not hand-write .scene JSON.
---

# rtype-author

Call the **rtype-author** MCP tools. The server is `RTypeEditor mcp` (not a Python script, and not a socket inside the open window). Do not edit `.scene` or `.prefab` JSON by hand.

## Workflow

1. `project_summary` then `scene_outline` on the game folder (`Games/Menu`, `R-Type`, …).
2. For UI: `list_ui_templates`, then `add_ui_screen` / `add_ui_widget` / `set_ui_props`. A button caption is `props.label`. The prop `text` is a color `[r, g, b, a]`, not the caption. Pass catalog ids and prop values only.
3. For entities: `create_entity`, `get_component`, `set_component`, `place_prefab`. `get_component` reads fields such as position `x`/`y`. Allowed components: position, rotation, name, sprite, collider, shape, text, opacity, camera, audio, input, network.
4. For waves: `add_wave`, `set_wave` with `wave.index`, `remove_wave` with `wave.index` or `wave.name`.
5. For prefabs: `create_prefab` from an entity (does not overwrite). `set_prefab_component` edits an existing `.prefab` and its scene instances.
6. For behavior: `attach_script` with hook bodies only (`onUpdate`, `onCollisionEnter`, …). The tool writes `local Script = {}` / `return Script`.
7. `lua_api` with one topic (`entity`, `component/sprite`) when a hook needs a function name.
6. Tell the user to **reopen the scene** in the editor. Do not ask them to Save over the tool’s write.

## Do not

- Invent `entityId`, `Parent`, or scene JSON keys.
- Dump the whole Lua manual; fetch one `lua_api` page.
- Overwrite an existing script by sending `hooks` when the file already exists; use a new `scriptPath` or attach without hooks.
