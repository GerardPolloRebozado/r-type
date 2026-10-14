# `Time`

The `Time` namespace exposes clock information. Prefer the `dt` argument handed to `onUpdate(dt)` for movement; `Time` is useful for effects, timers and non-movement logic.

## Functions

### `Time.deltaTime()`

Frame delta time (seconds). Currently returns a constant `1/60`.

**Signature:** `Time.deltaTime() -> number`

---

### `Time.time()`

Elapsed time since the program started (seconds).

**Signature:** `Time.time() -> number`

**Example**

```lua
if Time.time() > 10 then log.info("ten seconds have passed") end
```

---

## Related

- `onUpdate(dt)` — the per-frame hook receives `dt` directly.
- `onCreate(dt)` — also receives a delta time.