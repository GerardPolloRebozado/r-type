# Lua lifecycle and debugging

## Lifecycle

A `ScriptComponent` points at a Lua file. The script must `return` a table. The engine
calls, if defined:

- `onCreate(entity)`  -  once, when the script is loaded.
- `onUpdate(entity, dt)`  -  every frame, `dt` in seconds.
- `onDestroy(entity)`  -  when the entity is destroyed.
- `onCollisionEnter/Stay/Exit(self, other)` and `onTriggerEnter/Stay/Exit(self, other)`  - 
  when connected via `Component.connect`.

Each entity gets its own table, so `self` is never shared between entities. Store per-entity
state on `self` in `onCreate`.

## Load order

Scripts load during the game's first frames. `ScriptSystem` stays idle in the editor (the
`EditorMode` resource is present), so Lua only runs in the game process started by Play.

## Auto-reload

`ScriptComponent.autoReload` defaults to `true` and reloads the file when it changes on
disk. There is no hot reload inside a running process: save the file, close the game
window, and click Play again.

## Where logs go

`log.info`, `log.error`, `log.debug`, and `print` write to the engine's log buffer. In the
game process this appears in the terminal where you launched it (or the Console if launched
from the editor, since build/play output lands there).

## Debugging tips

- If nothing happens, check that the game actually ran (click Play, then click the game
  window). The editor never runs Lua.
- A Lua error prints to the log with the file and line; read the terminal/console.
- Use `log.info(tostring(value))` to inspect values.
- `Entity.find("Name")` returns `nil` if no entity has that `Name`; guard with `if`.
- `Time.deltaTime()` is a fallback `1/60`; prefer the `dt` argument.
- `Scene.load` is ignored on clients in multiplayer.

## What runs where (multiplayer)

| Call | Server | Client | Single-player |
|---|---|---|---|
| `onCreate` | yes | yes | yes |
| `onUpdate` (guarded by `shouldSimulate`) | yes | no | yes |
| `Net.isServer()` | true | false | false |
| `Net.isNetworked()` | true | true | false |
| `Scene.load` | applies | ignored | applies |
| `Net.input(entity)` for a player | that player's input | local player only | controller input |
