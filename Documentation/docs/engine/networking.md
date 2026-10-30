# Networking

Multiplayer in this engine is **server-authoritative**. That is a fancy way of saying: one
machine is the referee, everyone else is a player. The referee owns the board and decides
what is true. Players may only say "I press left". The referee moves the piece and
announces where everything is. Nobody moves their own piece on their own board.

That single idea explains why two players see the same world, and why cheating by editing
your own copy does nothing.

## Roles

Every process is in exactly one role, decided at startup.

| Role | Who simulates the world | Sends | Applies |
|---|---|---|---|
| Single-player | this process | nothing | nothing |
| Server | this process (authoritative) | spawn / state / despawn |  -  |
| Client | nobody, for networked entities | input | spawn / state / despawn |

A single-player game is **not** "multiplayer with one player". There is no host at all, and
replication does nothing. This is on purpose: a single-player build never pays for
networking it does not use, and one script can support all three modes by checking
`Net.isNetworked()`.

## Choosing a role

```bash
Game                       # single-player
Game --server              # server on port 3000
Game --server --port 4000  # server on another port
Game --server --headless   # server with no window
Game --client              # connect to 127.0.0.1:3000
Game --client --host 10.0.0.5 --port 4000
```

With no flag, no host is created and the scene runs exactly as before. The editor's Play
Multiplayer button launches these commands for you, so you do not need a terminal.

ENet is initialised lazily and exactly once inside `NetworkHostResource`. You never call
`enet_initialize` yourself.

## The only thing you write: `NetworkComponent`

Tag an entity with a `NetworkComponent` and the engine handles identity, spawning, input, and
state sync.

| Field | Type | Meaning |
|---|---|---|
| `networkId` | `uint64_t` | Engine-assigned identity. `0` means "not assigned yet". Never serialized. |
| `authority` | `string` | `"Server"`, `"Client"`, or `"Shared"` (see below). |
| `syncTransform` | `bool` | Sync `Position` + `Rotation` every tick. |
| `syncSprite` | `bool` | Sync the `SpriteComponent` (path, offsets, scale, flips). |
| `prefabPath` | `string` | Project-relative prefab to instantiate on the other machine. |

### Authority, in plain words

- **Server**: the server simulates it; clients follow. Enemies, pickups, platforms.
- **Client**: one client sends input; **the server still simulates it**. Each connected
  player. This is the one that trips people up: `"Client"` does not mean "the client
  simulates this". The client only contributes input; the server runs the movement and
  broadcasts the result.
- **Shared**: not synced at all; built identically from the scene. Walls, background.

There is no "both". If both machines simulated one entity, they would disagree the moment
anything non-deterministic happened, and the next snapshot would snap the loser back. That
rubber-banding is exactly what authority exists to prevent.

## How your script stays one file

Every replicated script guards its movement with the same predicate:

```lua
local function shouldSimulate()
    if not Net.isNetworked() then return true end   -- single-player: I am the only machine
    return Net.isServer()                          -- multiplayer: only the server simulates
end
```

One script, three outcomes:

| Process | `Net.isNetworked()` | `Net.isServer()` | `shouldSimulate()` |
|---|---|---|---|
| Single-player | false | false | **true** |
| Server | true | true | **true** |
| Client | true | false | **false** |

A client script is not useless: it still runs `onCreate` / `onDestroy`, RPC handlers, and
purely local effects (a muzzle flash, a HUD update). It only skips the part that *decides*
the entity's position.

## Frame order, and why it matters

`Script` runs before `Replication`, so Lua moves the player first and Replication
broadcasts the new position in the same tick. `Multiplayer` (the ENet poll) runs last, so
packets received this tick are dispatched immediately.

`ReplicationPlugin` must register before `MultiplayerPlugin`, otherwise the very first
poll could deliver a packet to a router with no handler yet.

## The plumbing (short version)

- `NetworkHostResource` is the only class that knows about ENet.
- `PacketRegistryResource` hashes packet names to deterministic 16-bit ids, so a Windows
  client and a Linux server agree on `"replication_spawn"` without sharing a table.
- `PacketRouterResource` dispatches packets to handlers.
- `PrefabSpawner` is a function pointer installed by the serializer, used to spawn prefabs
  at runtime on the server and clients.

Packets are `[2 bytes id, little-endian][payload]`. Spawns, despawns, and identity are
reliable; state and input are unreliable because the next snapshot supersedes them.

## Limits (read these before designing)

- Maximum 5 connected players.
- No client-side prediction: clients do not move their own player locally, so there is a
  tick of latency.
- No prefab instantiation from Lua; the engine spawns players.
- Clients follow the server's scene loads; their own `Scene.load` calls are ignored.

## Diagnostics

Set `RTYPE_NET_DEBUG=1` for a process to log per-second input and position lines.

## Lua API

Scripts use the `Net` namespace: `isNetworked`, `isServer`, `isLocal`, `localId`,
`playerSlot`, `replicate`, `rpc`, `rpcUnreliable`, `broadcast`, `on`, `off`, `register`,
`onConnect`, `onDisconnect`, `send`, `input`, and more. See
[Reference → Net](../lua/net.md).

## See also

- [Editor → Multiplayer](../editor/multiplayer.md)  -  the launch workflow.
- [How do I...? → Add multiplayer](../how-do-i/add-multiplayer.md).
- [Lua cookbook](../how-do-i/lua-cookbook.md).
