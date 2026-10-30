# Entity

## `Entity.new`

```lua
Entity.new(id) -> Entity
```

Coerces to an entity handle; `0` if no match.

## `Entity.create`

```lua
Entity.create() -> Entity
```

Creates a new entity in the registry.

## `Entity.destroy`

```lua
Entity.destroy(entity) -> nil
```

Destroys immediately.

## `Entity.queueDestroy`

```lua
Entity.queueDestroy(entity) -> nil
```

Queues destruction for end of frame.

## `Entity.find`

```lua
Entity.find(name) -> Entity | nil
```

First entity whose `Name` equals `name`.

## `Entity.getName`

```lua
Entity.getName(entity) -> string
```

Name value or `""` if no `Name`.

## `Entity.setName`

```lua
Entity.setName(entity, name) -> nil
```

Adds `Name` if missing, else updates.
