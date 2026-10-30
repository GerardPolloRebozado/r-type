# Layers

## `Layers.name`

```lua
Layers.name(layer) -> string
```

## `Layers.index`

```lua
Layers.index(name) -> int
```

## `Layers.bit`

```lua
Layers.bit(layer) -> uint32
Layers.bit(name) -> uint32
```

`Layers.bit("Unknown")` logs an error and returns 0.

## Values

- `Layers.all` is `0xFFFFFFFF`.
- `Layers.none` is `0`.
- `Layers.count` is `32`.

Layer names (0..7): `Default`, `Player`, `Enemy`, `PlayerBullet`, `EnemyBullet`, `Wall`,
`Pickup`, `Trigger`; others `Layer N`.
