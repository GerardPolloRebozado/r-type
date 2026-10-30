# Connections (signals)

Known signals: `onCollisionEnter`, `onCollisionStay`, `onCollisionExit`, `onTriggerEnter`,
`onTriggerStay`, `onTriggerExit`.

## `Component.connect`

```lua
Component.connect(emitter, signal, target, method) -> bool
```

Returns `true` if a duplicate already exists.

## `Component.disconnect`

```lua
Component.disconnect(emitter, signal, target?, method?) -> bool
```

Filters by the provided `target` and `method`.

## `Component.getConnections`

```lua
Component.getConnections(entity) -> { {signal, target, method}, ... }
```

1-indexed list of connection records.
