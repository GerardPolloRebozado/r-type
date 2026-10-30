# Cross-component query

## `Component.has`

```lua
Component.has(entity, typeName) -> bool
```

Returns whether the entity has a component of the given alias type.

## `Component.remove`

```lua
Component.remove(entity, typeName) -> nil
```

Removes the component of the given alias type from the entity.

## `Component.entitiesWith`

```lua
Component.entitiesWith(typeName) -> Entity[]
```

Returns a 1-indexed list of every entity that has a component of the given alias type.
