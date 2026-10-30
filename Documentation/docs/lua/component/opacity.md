# Opacity

## `Component.setOpacity`

```lua
Component.setOpacity(entity, alpha, inherit?) -> bool
```

Alpha is clamped to 0..1. If `inherit` is provided, uses `Opacity::Create(clamped, inherit)`;
otherwise, if the component exists, updates alpha while preserving inherit; otherwise uses
`Opacity::Create(clamped)`. Requires an alive entity.

## `Component.setOpacityInherit`

```lua
Component.setOpacityInherit(entity, inherit) -> bool
```

## `Component.getOpacityInherit`

```lua
Component.getOpacityInherit(entity) -> bool | nil
```

## `Component.getOpacity`

```lua
Component.getOpacity(entity) -> number | nil
```
