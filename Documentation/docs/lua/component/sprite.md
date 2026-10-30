# Sprite

## `Component.addSprite`

```lua
Component.addSprite(entity, path, offsetX?, offsetY?, scaleX?, scaleY?) -> bool
```

Ensures a `Position` (with rotation) and loads the texture via `AssetManager`. Logs on
failure.

## `Component.setSpriteRect`

```lua
Component.setSpriteRect(entity, x, y, width, height) -> bool
```

## `Component.getSprite`

```lua
Component.getSprite(entity) -> "sprite" | nil
```

Returns the literal string `"sprite"` if a sprite exists, else `nil`.

## `Component.getSpriteInfo`

```lua
Component.getSpriteInfo(entity) -> {path, offsetX, offsetY, scaleX, scaleY, flipX, flipY} | nil
```

## `Component.getSpriteFlip`

```lua
Component.getSpriteFlip(entity) -> {flipX, flipY} | nil
```

## `Component.setSpriteFlip`

```lua
Component.setSpriteFlip(entity, flipX, flipY?) -> bool
```

If `flipY` is omitted, only `flipX` changes.

## `Component.getSpriteScale`

```lua
Component.getSpriteScale(entity) -> {x, y} | nil
```

## `Component.getSpriteOffset`

```lua
Component.getSpriteOffset(entity) -> {x, y} | nil
```

## `Component.getSpritePath`

```lua
Component.getSpritePath(entity) -> string | nil
```

## `Component.setSpriteScale`

```lua
Component.setSpriteScale(entity, scaleX, scaleY) -> bool
```

## `Component.setSpriteOffset`

```lua
Component.setSpriteOffset(entity, offsetX, offsetY) -> bool
```

## `Component.setSpritePath`

```lua
Component.setSpritePath(entity, path) -> bool
```

## `Component.findEntityByName`

```lua
Component.findEntityByName(name) -> Entity | nil
```
