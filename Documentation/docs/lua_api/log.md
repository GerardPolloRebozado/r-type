# `log`

The `log` namespace provides lightweight logging helpers. All output is routed through the Engine's `Logger` so scripts and C++ share the same channels.

## Functions

### `log.info(message)`

Print an informational message.

**Signature:** `log.info(message: string)`

**Parameters**

| Name | Type | Description |
|------|------|-------------|
| `message` | `string` | Text to log. |

**Returns:** `nil`

**Example**

```lua
log.info("Player spawned at " .. x .. ", " .. y)
```

---

### `log.error(message)`

Print an error-level message.

**Signature:** `log.error(message: string)`

**Parameters**

| Name | Type | Description |
|------|------|-------------|
| `message` | `string` | Text to log. |

**Returns:** `nil`

**Example**

```lua
log.error("Failed to load sprite: " .. path)
```

---

### `log.debug(message)`

Print a debug-level message.

**Signature:** `log.debug(message: string)`

**Parameters**

| Name | Type | Description |
|------|------|-------------|
| `message` | `string` | Text to log. |

**Returns:** `nil`

**Example**

```lua
log.debug("deltaTime = " .. Time.deltaTime())
```

---

## Global `print(...)`

`print` collects all arguments, joins them with tabs and logs the result at the info level — useful for quick ad-hoc debugging.

**Signature:** `print(...: any)`

**Example**

```lua
print("entity", id, "hp =", hp)
```