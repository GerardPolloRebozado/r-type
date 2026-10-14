# Lua API Reference

This is the definitive reference for the Engine's built-in Lua scripting API. It mirrors the namespaces registered by `Scripting::LuaAPI::registerEngineAPI` and is kept in sync with the source files under `Engine/src/Scripting/LuaAPI/`.

!!! note "Developer Guide"
    The handwritten developer guide lives at [Lua Scripting](../engine/lua_scripting.md). This section is the flat, exhaustive reference — one page per namespace.

## Namespaces

| Namespace | Purpose |
|-----------|---------|
| [`log`](log.md) | Logging helpers (`info`, `error`, `debug`) and the global `print`. |
| [`Entity`](entity.md) | Create, destroy, find and rename entities. |
| [`Transform`](transform.md) | Read/write position and rotation of entities. |
| [`Time`](time.md) | Frame delta and elapsed time. |
| [`Input`](input.md) | Keyboard and mouse polling, plus the `KEY.*` constants. |
| [`Component`](component.md) | Add, query and remove components on entities. |
| [`Scene`](scene.md) | Scene-level operations (currently a placeholder). |

## Globals

| Name | Type | Description |
|------|------|-------------|
| `print` | `function(...)` | Print any number of values (tab-separated) as an info log. |
| `KEY_*` constants | `integer` | Exposed under the global `KEY` table (see [Input](input.md)). |
---

Every function is registered in `Engine/src/Scripting/LuaAPI/` — when you add a new binding, add the matching entry here so the documentation stays in sync.