# Transform

Combines `Position` + `Rotation` + draw layer (z).

## `Transform.getPosition`

```lua
Transform.getPosition(entity) -> {x, y, z}
```

Returns `{0, 0, 0}` if no Position.

## `Transform.setPosition`

```lua
Transform.setPosition(entity, x, y, z?) -> nil
```

Adds Position if missing. `z` is optional and keeps the current z when present, else 0.

## `Transform.translate`

```lua
Transform.translate(entity, dx, dy) -> nil
```

Adds Position if missing. Only x and y; z untouched.

## `Transform.getZIndex`

```lua
Transform.getZIndex(entity) -> int
```

Returns `0` if no Position.

## `Transform.setZIndex`

```lua
Transform.setZIndex(entity, z) -> nil
```

Adds Position `{0, 0, z}` if missing.

## `Transform.getRotation`

```lua
Transform.getRotation(entity) -> number
```

Returns `0.0` if no Rotation.

## `Transform.setRotation`

```lua
Transform.setRotation(entity, angle) -> nil
```

Adds Rotation if missing.

## `Transform.rotate`

```lua
Transform.rotate(entity, deltaAngle) -> nil
```

Adds Rotation(delta) if missing, else `+=`.
