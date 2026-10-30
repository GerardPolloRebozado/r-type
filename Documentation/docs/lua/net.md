# Net (Multiplayer)

## `Net.register`

```lua
Net.register(name) -> int | nil
```

## `Net.id`

```lua
Net.id(name) -> int | nil
```

Lookup only.

## `Net.name`

```lua
Net.name(id) -> string | nil
```

## `Net.startServer`

```lua
Net.startServer(port, maxClients?) -> bool
```

`maxClients` defaults to 32.

## `Net.startClient`

```lua
Net.startClient(host, port) -> bool
```

## `Net.isServer`

```lua
Net.isServer() -> bool
```

## `Net.isNetworked`

```lua
Net.isNetworked() -> bool
```

## `Net.localId`

```lua
Net.localId() -> number | nil
```

## `Net.localEntity`

```lua
Net.localEntity() -> Entity | nil
```

## `Net.maxPlayers`

```lua
Net.maxPlayers() -> int
```

## `Net.on`

```lua
Net.on(name, callback) -> bool
```

Registers a packet handler. The callback receives `(peer, data)`. `data` is a Lua table for
JSON payloads, a raw string otherwise, or `nil` for empty payloads.

## `Net.off`

```lua
Net.off(name) -> bool
```

## `Net.send`

```lua
Net.send(peer, name, data, reliable?, channel?) -> bool
```

`peer` may be `void*` or `ENetPeer*`. `reliable` defaults to `false`, `channel` to 0.

## `Net.broadcast`

```lua
Net.broadcast(name, data, reliable?, channel?) -> bool
```

`reliable` defaults to `false`, `channel` to 0.

## `Net.onConnect`

```lua
Net.onConnect(callback or nil) -> nil
```

Pass `nil` to clear.

## `Net.onDisconnect`

```lua
Net.onDisconnect(callback or nil) -> nil
```

Pass `nil` to clear.

## `Net.peerEntity`

```lua
Net.peerEntity(peer) -> Entity | nil
```

## `Net.networkId`

```lua
Net.networkId(entity) -> number | nil
```

## `Net.playerSlot`

```lua
Net.playerSlot(entity) -> int | nil
```

## `Net.entityFromId`

```lua
Net.entityFromId(networkId) -> Entity | nil
```

## `Net.track`

```lua
Net.track(entity, networkId) -> bool
```

## `Net.untrack`

```lua
Net.untrack(entity) -> bool
```

## `Net.isLocal`

```lua
Net.isLocal(entity) -> bool
```

## `Net.input`

```lua
Net.input(entity) -> {[action]: true} | nil
```

Server: last input for that player. Client: local controller input if the entity is local,
else `nil`.

## `Net.replicate`

```lua
Net.replicate(entity, authority?, syncTransform?, syncSprite?, prefabPath?) -> bool
```

`authority` defaults to `"Server"`, `syncTransform` to `true`, `syncSprite` to `false`. Tags
an entity for replication.

## `Net.rpc`

```lua
Net.rpc(entity, method, data) -> bool
```

Reliable entity RPC.

## `Net.rpcUnreliable`

```lua
Net.rpcUnreliable(entity, method, data) -> bool
```

Unreliable variant.

## `Net.onEntityRpc`

```lua
Net.onEntityRpc(method, handler) -> bool
```

Handler receives `(entity, data)`.

## Payload encoding

- `Net.on` callback `data`: JSON payloads become Lua tables; non-JSON becomes a raw
  string; empty payloads become `nil`.
- `EncodePayload`: strings are sent raw (no JSON quoting); tables are serialized to JSON;
  functions/userdata become null; cycles and null are guarded.
