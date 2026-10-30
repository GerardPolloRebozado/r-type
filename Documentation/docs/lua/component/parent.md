# Parent / Hierarchy

## `Component.setParent`

```lua
Component.setParent(child, parent, relative?) -> bool
```

`relative` defaults to `true`.

## `Component.getParent`

```lua
Component.getParent(entity) -> Entity | nil
```

## `Component.getChildren`

```lua
Component.getChildren(entity) -> Entity[]
```

1-indexed list of direct children.

## `Component.flipX`

```lua
Component.flipX(entity) -> bool
```

Mirrors the entity and its child subtree about the entity's centre.
