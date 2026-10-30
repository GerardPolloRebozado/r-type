# Logging

Engine and editor code record messages with the macros in `Logging/Logger.hpp`. The caller does not write to the terminal itself.

## Terminal

`log_error`, `log_debug`, `log_info`, and `log_user_error` move the line onto a queue and return. One background thread writes the batch, then flushes. `[ERROR]` goes to standard error. `[DEBUG]` and `[INFO]` go to standard output.

That thread is per process. A game started with Play has its own writer. Those lines stay on that process's terminal. They are not piped into the editor.

The web build has no worker thread. There, each line is written on the caller.

## Editor Console

The Console panel lists user problems only. A line appears there when code calls `log_user_error`. That is a failure of something the user controls: a script, a prefab, a scene file, a path, or a drag that the hierarchy rejects.

`log_error`, `log_debug`, and `log_info` are engine and editor diagnostics. They stay on the terminal. An empty Console means there is no user problem to show.

## Lua

Scripts use the `log` table and the global `print`.

| Call | Where it shows |
|------|----------------|
| `log.error("...")` | Editor Console and the terminal |
| `log.info("...")` | Terminal only |
| `log.debug("...")` | Terminal only |
| `print(...)` | Terminal only |

```lua
log.error("missing player spawn")
log.info("wave started")
print("tick", 3)
```
