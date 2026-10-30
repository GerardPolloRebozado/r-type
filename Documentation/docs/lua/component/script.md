# Script

## `Component.addScript`

```lua
Component.addScript(entity, path, autoReload?) -> bool
```

Requires an alive entity and a non-empty path. Replaces the script if the path differs.
`autoReload` defaults to `true`.

## `Component.getScriptPath`

```lua
Component.getScriptPath(entity) -> string | nil
```
