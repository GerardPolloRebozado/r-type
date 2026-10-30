# Set up input mapping

## Create an action

Open **Input Map** from the menu bar. Type an action name (for example `jump`) and click
**+ Add Action**. Action names are case-sensitive: `jump`, `Jump`, and `JUMP` are three
different actions.

## Bind keys

Expand the action, click **+ Add Keybind**, then click the **Key** button and press the
desired key. You can bind several keys to one action (for example `W` and `Up` both map to
`MOVE_UP`). You can also bind mouse buttons and gamepad buttons.

## Read the action in Lua

```lua
if Controller.isActionActive("jump", 1) then
    -- player 1 pressed jump
end
```

Player index defaults to 1. For local multiplayer, add a second player and read
`Controller.isActionActive("jump", 2)`.

## How it is stored

The map is stored in the scene as `m_InputMap` and baked into the game launcher at build
time. Changing it requires a rebuild before the game sees the new bindings.

## Tips

- Prefer action names over raw keys in scripts; rebinding then needs no code changes.
- Use `Controller.getActiveActions(playerId)` to inspect what is currently held.
- Custom devices: subclass `IController` and feed `_activeActions` from your source
  (network, AI, replay).

See [Engine → Input](../engine/input.md), [Editor → Input mapping](../editor/input.md), and
[Lua → Input & Controller](../lua/input.md).
