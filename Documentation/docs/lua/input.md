# Input & Controller

## `Input.isKeyDown`

```lua
Input.isKeyDown(key) -> bool
```

## `Input.isKeyPressed`

```lua
Input.isKeyPressed(key) -> bool
```

## `Input.isKeyReleased`

```lua
Input.isKeyReleased(key) -> bool
```

## `Input.getMousePosition`

```lua
Input.getMousePosition() -> {x, y}
```

`KEY.*` constants (raylib key codes): `SPACE`, `APOSTROPHE`, `COMMA`, `MINUS`, `PERIOD`,
`SLASH`, `ZERO`..`NINE`, `SEMICOLON`, `EQUAL`, `A`..`Z`, `LEFT_BRACKET`, `BACKSLASH`,
`RIGHT_BRACKET`, `GRAVE_ACCENT`, `ESCAPE`, `ENTER`, `TAB`, `BACKSPACE`, `INSERT`,
`DELETE`, `RIGHT`, `LEFT`, `DOWN`, `UP`, `PAGE_UP`, `PAGE_DOWN`, `HOME`, `END`,
`CAPS_LOCK`, `SCROLL_LOCK`, `NUM_LOCK`, `PRINT_SCREEN`, `PAUSE`, `F1`..`F12`,
`LEFT_SHIFT`, `LEFT_CONTROL`, `LEFT_ALT`, `RIGHT_SHIFT`, `RIGHT_CONTROL`, `RIGHT_ALT`.

## `Controller.hasController`

```lua
Controller.hasController(playerId?) -> bool
```

`playerId` defaults to 1.

## `Controller.isActionActive`

```lua
Controller.isActionActive(action, playerId?) -> bool
```

`playerId` defaults to 1.

## `Controller.getActiveActions`

```lua
Controller.getActiveActions(playerId?) -> string[]
```

1-indexed list. `playerId` defaults to 1.

## `Controller.addKeybind`

```lua
Controller.addKeybind(action, key, device?, composeKey?, playerId?, networked?) -> bool
```

`device` defaults to `KEYBOARD` (0), must be 0..3. `composeKey` defaults to 0. `playerId`
defaults to 1. `networked` defaults to `false`. Creates the `InputManager`/`Controller` if
missing.

Constants: `Controller.KEYBOARD = 0`, `MOUSE = 1`, `GAMEPAD = 2`, `TOUCH = 3`.
