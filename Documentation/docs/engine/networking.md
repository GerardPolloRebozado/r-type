# Networking & Multiplayer

The engine ships a **declarative, server-authoritative** multiplayer layer. You do not write
sockets, handshakes or state diffs in your gameplay code: you tag an entity with a
`NetworkComponent`, and the engine handles identity, spawning, input and state sync for you.
Gameplay scripts keep doing what they always did — move, shoot, animate — and only ask the
engine *"what input does this player have?"*.

> **In plain words:** think of a tabletop game with a referee. The **server** is the referee: it
> owns the board and decides what is true. Each **client** is a player who may only say *"I press
> left"*. The referee moves the piece and announces where everything is. Nobody moves their own
> piece on their own board. That is why two players see the same world, and why cheating by
> editing your own copy does nothing.

This page explains how that is implemented under the hood. For the tools you use to *author* a
multiplayer game, see [Play Multiplayer](../editor/multiplayer.md), and for ready-made
[recipes for different genres](../editor/multiplayer.md#7-recipes-for-different-genres).

---

## What these words mean

A few words are used so often below that they are worth pinning down first.

**Replication** is keeping a *copy* of an entity's state on every machine in sync with the machine
that owns it. When the server moves an enemy, replication is the machinery that makes that same enemy
appear in the same place in every client's window. **Declarative** replication means you declare
*what* to keep in sync (the two checkboxes on `NetworkComponent`) and the engine decides *how*.
Hand-writing the packets yourself is the *imperative* alternative — that is what the low-level
`Net.send` / `Net.on` API is for.

**Authority** is *which machine gets to decide* an entity's state. "Authoritative" means "the one
whose version wins". This engine is **server-authoritative**: the server's version of the world is the
truth, and clients are told about it.

**To simulate** an entity means to run the gameplay logic that changes it — the `onUpdate` that moves
it, the AI that steers it, the collision that damages it. Only *some* machines should simulate any
given entity; that is what authority selects (see [§4](#the-authority-model)).

**Snapshot** is the bundle of state the server sends each tick for every replicated entity. Here it is
a JSON array, sent every tick, replaced wholesale by the next one.

**RPC** (remote procedure call) is a one-off custom message ("play this card", "open this door"), used
when an *event* matters but continuous state does not.

---

## 1. The three roles

Every process that runs a scene is in exactly one role. The role is decided once, at startup.

| Role | Who simulates the world | Sends | Applies |
|---|---|---|---|
| **Single-player** | this process | nothing | nothing |
| **Server** | this process (authoritative) | spawn / state / despawn | — |
| **Client** | nobody for networked entities | input | spawn / state / despawn |

A **single-player** game is not "multiplayer with one player": there is no network host at all, and
the replication system does nothing. This is intentional — a single-player build never pays for
networking it does not use, and one script can support both modes by checking `Net.isNetworked()`.

---

## 2. Choosing a role at startup

The role comes from command-line flags parsed by `MultiplayerPlugin::Init`
(`Engine/src/Plugins/MultiplayerPlugin.cpp`, `StartHostFromLaunchOptions`):

```bash
Game                       # single-player: no host, scene runs as-is
Game --server              # server on port 3000
Game --server --port 4000  # server on another port
Game --server --headless   # server with no window / no rendering
Game --client              # connect to 127.0.0.1:3000
Game --client --host 10.0.0.5 --port 4000
```

| Flag | Meaning | Default |
|---|---|---|
| `--server` | create an authoritative server host | — |
| `--client` | create a client host and connect | — |
| `--host <addr>` | server address for `--client` | `127.0.0.1` |
| `--port <n>` | port to listen on / connect to | `3000` |
| `--headless` | run with no window (server) | off |

With no `--server` / `--client` flag, **no host is created** and the scene behaves exactly as it did
before multiplayer existed.

ENet is initialised **lazily and exactly once** inside `NetworkHostResource` (a `std::call_once`
guarded `enet_initialize`). The generated game entry point never calls `enet_initialize` itself, so
you never have to think about ENet lifetime.

The [editor's Play Multiplayer button](../editor/multiplayer.md) launches exactly these commands,
which is why a multiplayer playtest needs no terminal.

---

## 3. Architecture at a glance

Two plugins wire everything up. In the generated `main.cpp`, **`ReplicationPlugin` is registered
before `MultiplayerPlugin`** — this order matters (see [§5](#5-the-frame-loop-and-why-order-matters)).

| Piece | File | Responsibility |
|---|---|---|
| `ReplicationPlugin` | `Engine/src/Plugins/ReplicationPlugin.cpp` | registers `NetworkComponent` + `ReplicationSystem` |
| `MultiplayerPlugin` | `Engine/src/Plugins/MultiplayerPlugin.cpp` | registers resources, parses CLI, creates the host, registers `MultiplayerSystem` |
| `ReplicationSystem` | `Engine/src/Systems/ReplicationSystem.cpp` | the brain: init, connect, input, state, spawn, despawn |
| `MultiplayerSystem` | `Engine/src/Systems/MultiplayerSystem.cpp` | the pipe: polls the host once per tick |
| `NetworkHostResource` | `Engine/src/Resources/NetworkHostResource.cpp` | the only file that knows ENet exists |
| `PacketRegistryResource` | `Engine/include/Resources/PacketRegistryResource.hpp` | deterministic `name → packet id` |
| `PacketRouterResource` | `Engine/include/Resources/PacketRouterResource.hpp` | `packet id → handler` dispatch |
| `PlayerInputState` | `Engine/include/Resources/PlayerInputState.hpp` | server-side `networkId → last input` |
| `PlayerSlotState` | `Engine/include/Resources/PlayerSlotState.hpp` | `networkId → color slot` (max 5) |
| `PrefabSpawner` | `Engine/include/Resources/PrefabSpawner.hpp` | runtime prefab instantiation hook |
| `NetworkComponent` | `Engine/include/Components/NetworkComponent.hpp` | per-entity replication metadata |
| `PlayerInformation` | `Engine/include/Resources/PacketRouterResource.hpp` | `{ id }`, the entity's network id |

The **editor process** itself initialises `ReplicationPlugin` (see `Editor/main.cpp`) — that
registers `NetworkComponent` so the Inspector can show and edit the **Network** section. It never
initialises `MultiplayerPlugin`, so the editor never creates a host and never runs a scene.

### A note on `PrefabSpawner` (dependency inversion)

The engine module cannot depend on the serialization module, because serialization already depends
on the engine. But the replication system needs to spawn `.prefab` files at runtime. The solution is
a function pointer installed on the registry by `Serialization::SceneSerializer` when it
deserializes a scene:

```cpp
struct PrefabSpawner {
    std::function<Entity(Registry&, const std::string&)> Spawn;
};
```

Every editor-generated game loads scenes through the serializer, so it gets runtime prefab
spawning for free — `ReplicationSystem` just calls `Spawn(registry, path)`.

This hook is **C++-only**, and it is exactly how the players get created:

- the **server** calls it once per connection to spawn `m_playerPrefabPath` (the player prefab);
- a **client** calls it when a `replication_spawn` / `replication_state` message carries a
  `prefabPath`, to materialise that prefab locally.

There is currently **no Lua equivalent**. A Lua script can only `Entity.create()` a *bare* entity and
dress it with `Component.addSprite` / `addBoxCollider` / `addScript` / …; it cannot instantiate a
`.prefab`. See the [TODO in §14](#14-todos-and-planned-work). One workaround already exists: the
server builds its entity in Lua but passes a `prefabPath` to `Net.replicate(...)`, so the *clients*
still instantiate the authored prefab even though the server cannot.

---

## 4. The `NetworkComponent`

Attached to an entity (usually authored on a `.prefab`), this is the **only** thing a game
developer writes to make an entity networked.

| Field | Type | Meaning |
|---|---|---|
| `networkId` | `uint64_t` | engine-assigned identity. `0` = not assigned yet (never serialized). |
| `authority` | `std::string` | `"Server"` / `"Client"` / `"Shared"` (see below). |
| `syncTransform` | `bool` | sync `Position` (x, y, z) + `Rotation` (angle) every tick. |
| `syncSprite` | `bool` | sync the `SpriteComponent` (path, offsets, scale, flips). |
| `prefabPath` | `std::string` | project-relative prefab to instantiate on the other side. |

The `networkId` is **excluded from scene serialization** — it is identity, not authored data, so a
scene saved on the server cannot bake a stale id into the file.

### The authority model

`authority` answers one question: *which machine decides this entity's state?*

| Authority | Server does | Client does | Replicated | Typical use |
|---|---|---|---|---|
| `"Server"` | simulates it | renders the replicated result | yes (state) | enemies, bullets, pickups, moving platforms |
| `"Client"` | simulates it **using one client's input** | sends input, renders the replicated result | yes (state) | each connected player |
| `"Shared"` | builds it from the scene, never syncs | builds it from the scene, never syncs | **no** | walls, background, level geometry |

> **The most common misunderstanding:** `"Client"` does **not** mean "the client simulates this". The
> client only *contributes input*; the **server** still runs the movement and broadcasts the result.
> That is why a player never moves their own ship locally — and why there is a tick of latency (no
> prediction). Read it as *"server-owned, but driven by one client's input"*.

**What happens on each machine**

- **`Server`, on the server:** the gameplay script runs and moves it; `ReplicationSystem` broadcasts
  the new position/sprite every tick.
- **`Server`, on a client:** the script's `shouldSimulate()` is `false`, so it does nothing;
  `ReplicationSystem` overwrites the entity's position/sprite from the latest snapshot.
- **`Client`, on the server:** same as `Server`, except the server reads *that one player's* input
  (`Net.input(entity)`); the other players' input is ignored for this entity.
- **`Client`, on a client:** the local player does nothing but *send input*; remote players are pure
  replicas. No client ever moves any player.
- **`Shared`:** nothing is synced. Every machine builds the entity from the same scene and leaves it
  alone. Use it only for things that are identical and effectively static; two machines simulating a
  `Shared` entity independently will drift apart.

**There is no "both".** If both machines simulated one entity, they would disagree the moment anything
non-deterministic happened (a dropped frame, floating-point drift, a lost packet), and the next
snapshot would snap the loser back — visible as rubber-banding. Authority exists precisely to pick
**one** simulator. The "both want to control it" case is modelled as `"Client"`: the client supplies
input, the server is the single decider.

### Why scripts check `shouldSimulate`

Every gameplay script that mutates a replicated entity guards its `onUpdate`/collision logic with the
same predicate:

```lua
local function shouldSimulate()
    if not Net.isNetworked() then return true end   -- single-player: I am the only machine
    return Net.isServer()                            -- multiplayer: only the server simulates
end
```

**Why:** a client that also ran the movement would fight the server. It would move the enemy a little,
then the next snapshot would overwrite it — jitter, rubber-banding, and (for a player) double speed.
The guard makes **one script correct in all three roles**:

| Process | `Net.isNetworked()` | `Net.isServer()` | `shouldSimulate()` | Effect |
|---|---|---|---|---|
| Single-player | false | false | **true** | the script moves and animates normally |
| Server | true | true | **true** | the script simulates authoritatively |
| Client | true | false | **false** | the script is inert; snapshots drive the entity |

A client script is *not* useless, though: it still runs `onCreate` / `onDestroy`, RPC handlers, and
purely local effects (a muzzle flash, a HUD update). It only skips the part that *decides* the
entity's position and animation.


---

## 5. The frame loop and why order matters

`Application::Run` is a **fixed-timestep** loop (`targetFPS`, e.g. 60). Each step calls `Update(dt)`,
which runs the registered systems **in registration order**, then `FlushQueues()` runs once per
frame:

```
Per fixed tick (1/targetFPS seconds):
  Hierarchy → Position → Text → Sprite → Shape → Rotation → Collider
            → Opacity → Signal → Input → Audio → Camera
            → Script            (Lua onCreate / onUpdate: gameplay)
            → Replication       (initialize, events, movement, broadcast / send input)
            → Multiplayer       (ENet poll: receive events → dispatch handlers)
End of frame:
  Registry::FlushQueues()       (deferred entity destruction)
```

Two consequences drive the whole design:

1. **`Script` runs before `Replication`.** On the server, Lua moves the player first, then Replication
   snapshots and broadcasts the *new* position in the same tick. No one-tick delay is introduced by
   the ordering.
2. **`Multiplayer` (the poll) runs last.** Packets received this tick are dispatched immediately, and
   anything `ReplicationSystem` queued to send is flushed by ENet during the poll.

This is *why* `ReplicationPlugin` must register before `MultiplayerPlugin`:

> `ReplicationSystem` installs its packet handlers (`replication_state`, `replication_spawn`,
> `replication_despawn`, `player_input`) during its first `Update`. `MultiplayerSystem` polls
> afterwards. If the order were reversed, the very first poll could deliver a packet to a router
> that has no handler yet — the message would be silently dropped.

The imported game entry expresses this with an explicit comment:

```cpp
Engine::Multiplayer::ReplicationPlugin replicationPlugin;
replicationPlugin.Init(app.GetRegistry());
Engine::Multiplayer::MultiplayerPlugin multiplayerPlugin;
multiplayerPlugin.Init(app.GetRegistry());
```

---

## 6. Layer 1 — the transport (`NetworkHostResource`)

`NetworkHostResource` wraps an `ENetHost` and is the **only** class that includes ENet. Everything
above it speaks in named packets and typed payloads.

### Packet framing

Every packet on the wire is:

```
[ 2 bytes: packet id, little-endian ][ payload ]
```

The router reads the first two bytes, looks up the handler for that id, and calls it with the
payload. Two payload encodings coexist:

- **BEVE** (`glz::write_beve`, packet id encoded as an integral type) — compact binary, used for the
  SDK-owned `your_player_id` control message.
- **JSON** (`glz::write_json`, packet built with `CreateRawPacket`) — used for all replication and
  gameplay packets. Human-readable and easy to extend; the cost is irrelevant at this scale.

### Reliability and channels

Reliability is per-packet (`ENET_PACKET_FLAG_RELIABLE`):

| Packet | Reliable? | Why |
|---|---|---|
| `your_player_id` | ✅ | identity must arrive exactly once |
| `replication_spawn` | ✅ | a missed spawn is a permanently invisible entity |
| `replication_despawn` | ✅ | a missed despawn is a ghost |
| `replication_state` | ❌ | sent every tick; the next snapshot supersedes a lost one |
| `player_input` | ❌ | sent every tick while held; loss is invisible at 60 Hz |
| `entity_rpc` | ✅ or ❌ | chosen by the caller (`Net.rpc` / `Net.rpcUnreliable`) |

Hosts are created with 2 channels for future prioritisation (e.g. chat vs game traffic), but the
current code sends everything on **channel 0**.

### Packet ids are deterministic

`PacketRegistryResource::GetOrRegister("replication_state")` hashes the *name* with FNV-1a into a
16-bit id in `[200, 65535]`:

```cpp
inline PacketTypeID HashPacketName(std::string_view name) {
    std::uint32_t hash = 2166136261u;
    for (char c : name) { hash ^= (unsigned char)c; hash *= 16777619u; }
    // ids below UserDefinedStart (200) are reserved for the SDK
    return 200 + hash % (0xFFFF - 200 + 1);
}
```

Because the hash is a fixed algorithm rather than the standard library's (`std::hash` is
implementation-defined), a Windows client and a Linux server compute the *same* id for
`"replication_spawn"` without sharing a table. Ids below 200 are reserved for built-in packets
(`Ping`, `Connect`, `Disconnect`).

### The handshake: `your_player_id`

The SDK owns one control message so clients learn their identity without any game code:

```
Server, on ENet CONNECT                Client, on receiving your_player_id
──────────────────────────            ──────────────────────────────────
create placeholder entity              decode { id } → m_localNetworkId = id
networkId = nextNetworkId++
attach PlayerInformation{networkId}
peer->data = entity
send your_player_id{ networkId }  ───▶
queue ConnectionEvent{Connect}
```

The peer↔entity link is stored **in the peer itself** (`enet_peer_set_data`), so a disconnect can
recover the entity and its network id with `GetEntityID(peer)` even before any game bookkeeping.

### Connection events

Rather than reacting to ENet events inside the poll, `NetworkHostResource` **queues** them:

```cpp
struct ConnectionEvent {
    enum class Type { Connect, Disconnect };
    Type type;
    ENetPeer* peer;
    Entity entity;       // bound when raised
    std::size_t networkId;
};
```

`ReplicationSystem` drains this queue on its own tick. Decoupling the transport from gameplay keeps
the connect/disconnect logic in one predictable place.

### Polling

`MultiplayerSystem::Update` is three lines: if a host exists, call `host.Poll(0, router, registry)`
with a **zero timeout** (non-blocking). `Poll` drains *every* pending ENet event each tick and
dispatches received payloads through the router.

---

## 7. Layer 2 — replication (`ReplicationSystem`)

`ReplicationSystem` is the heart of the feature. It reads `NetworkComponent`s and drives the entire
lifecycle. Its `Update` does the following, in order:

```
if no host → return                       (single-player)
Initialize() once                          (templates, ids, announced set)
register packet handlers once              (state, spawn, despawn, input)
drain connection events → connect/disconnect handling
server: ApplyDefaultMovement + BroadcastState
client: SendLocalInput
```

### 7.1 Initialisation — three families of entities

On the first tick with a host, `Initialize` walks the scene once and sorts entities into three
families:

| Scene authority | What happens | Example |
|---|---|---|
| `"Client"` | remembered as the **player prefab** (`m_playerPrefabPath`) and **destroyed** — it was only a per-player template | `Player` |
| `"Server"` | `networkId = entity id`, tracked; server pre-seeds `m_announcedIds` | `Pata-Pata` enemy |
| `"Shared"` | ignored, left to every machine's own scene | walls |

Two subtleties worth calling out:

- **The `"Client"` template is removed on clients too**, not just the server. Otherwise a client
  would keep a frozen local player next to the server-spawned one. The server later spawns one real
  player per connection from `m_playerPrefabPath`.
- **Scene `"Server"` entities reuse their entity id as the network id.** Since both sides load the
  same scene, the client can match its local copy to an incoming snapshot with no spawn round-trip,
  which also preserves parent/child links (the enemy's trail children keep working).

After initialisation the server calls `host.SetNextNetworkId(1'000'000)`: **runtime spawns live above
the scene id range** so a bullet can never collide with a scene entity's id.

### 7.2 Connecting (server)

When `Poll` creates the placeholder and queues a `Connect`, `ProcessConnect` runs on the next
`ReplicationSystem` tick:

```cpp
1. ignore the peer if it already disconnected between two polls
2. assign a color slot           → PlayerSlotState::Assign(networkId)
3. spawn the player prefab       → SpawnFromPrefab(m_playerPrefabPath)
4. destroy the placeholder, rebind the peer to the real player
5. set NetworkComponent{ id, authority = "Client", syncTransform, syncSprite }
6. TrackNetworkEntity + SetPeerEntity
7. SendSpawn(...)                → reliable, includes the color slot
```

The **placeholder** exists only to carry the handshake and the peer binding during the few ticks
between ENet's CONNECT and the prefab spawn. Replacing it with the real player is invisible to game
code because input is stored by network id, not by entity (see 7.3).

### 7.3 Input: client → server → gameplay

This is the part that lets gameplay scripts stay ordinary.

**Client** (`SendLocalInput`, every tick):

```jsonc
{ "id": 1000000, "input": { "up": false, "down": false, "left": false, "right": true, "shoot": true } }
```

sent **unreliably** to the server.

**Server** (`RegisterInputHandler`): decodes it and stores it in `PlayerInputState.byNetworkId`.
It is keyed by **network id**, so the placeholder→player swap cannot lose the input.

**Gameplay**: a script calls `Net.input(entity)`:

```lua
local input = Net.input(self.entity)
if input ~= nil and input.right then
    Transform.translate(self.entity, speed * dt, 0)
end
```

`Net.input` is deliberately symmetric, so the *same* script supports all three roles:

| Role | `Net.input(entity)` returns |
|---|---|
| Single-player | `nil` → the script falls back to `Controller.isActionActive(...)` |
| Server | the last input received for that player's network id |
| Client | the local controller input **if** the entity is the local player, else `nil` |

### 7.4 State broadcast (server)

Every tick, `BroadcastState` walks all entities with a `NetworkComponent` (skipping `"Shared"`):

- A runtime entity with `networkId == 0` (created by a script) is assigned an id, tracked, and a
  `replication_spawn` is sent **before** its first state entry.
- A snapshot entry is built:

```jsonc
{
  "id": 1000001,
  "authority": "Client",
  "prefabPath": "Prefabs/Player.prefab",
  "slot": 1,
  "x": 118.0, "y": -12.5, "z": 0,
  "angle": 0.0,
  "sprite": { "path": "assets/player/sprite_008.png",
              "offsetX": -12.8, "offsetY": -14.0,
              "scaleX": 4.0, "scaleY": 4.0,
              "flipX": false, "flipY": false }
}
```

  `x/y/z/angle` appear only when `syncTransform`, `sprite` only when `syncSprite`, `slot` only for
  players.

- Finally, the set of ids broadcast this tick is **diffed against `m_announcedIds`** — everything
  the clients currently know about. Ids that are announced but absent have been destroyed and get a
  reliable `replication_despawn`.

> **Why diff against the announced set and not last tick's snapshot?** An entity created and
> destroyed *within a single tick* never appears in two consecutive snapshots. Diffing against a
> snapshot would miss it and leave a permanent ghost on every client. Diffing against everything
> ever announced catches it.

Everything is sent as **one unreliable JSON array** per tick. Full snapshots (rather than deltas)
keep the model simple and rewindable, which is what would make client-side prediction possible later.

### 7.5 Receiving (client)

Three handlers, one per packet type:

- **`replication_spawn`** (reliable): if the id is unknown, apply the color slot first, then
  `SpawnFromPrefab(prefabPath)` (or a bare entity if none was sent), attach a `NetworkComponent`,
  and track it.
- **`replication_state`** (unreliable): for each entry, resolve the entity by network id. If it is
  unknown, **spawn it lazily** from `prefabPath` — this is how a late joiner materialises the world —
  then apply position/rotation/sprite/slot.
- **`replication_despawn`** (reliable): untrack and destroy the replica.

Because spawn and despawn are reliable while state is not, a client can never end up with a
permanent ghost: the spawn always arrives, and the despawn diff always fires.

> **Ordering detail:** the color slot is applied *before* the prefab is instantiated, so the player
> prefab's `onCreate` already sees the right slot and picks the right sprite block on its very first
> frame.

### 7.6 Disconnecting

`ProcessDisconnect` destroys any leftover entity, drops the stale input (so a reused id never keeps
moving), **frees the color slot**, and sends the despawn immediately rather than waiting one tick for
the diff — so a disconnect feels instant to the other players.

### 7.7 Player colors (slots)

`PlayerSlotState` maps each connected player to a **0-based color slot**, `kMaxPlayers = 5`:

- The server assigns the **smallest free slot** on connect and frees it on disconnect.
- The slot travels in `replication_spawn` and `replication_state`, so clients know it too.
- `Net.playerSlot(entity)` reads it on either side; `Net.maxPlayers()` returns the cap.
- The transport itself enforces the cap: the server host is created with
  `maxClients = PlayerSlotState::kMaxPlayers`, so ENet rejects a sixth connection before the
  replication system ever sees it.

The R-Type game maps slot `N` to the sprite block `5N+1 .. 5N+5` (see `player.lua`), but the slot
mechanism itself is game-agnostic: any game can map a slot to a color, a model, a team, a spawn
point, etc.

---

## 8. Packet reference

| Packet | Direction | Reliability | Payload |
|---|---|---|---|
| `your_player_id` | server → client | reliable | `{ id }` (BEVE) |
| `replication_spawn` | server → all | reliable | `{ id, authority, prefabPath, slot? }` |
| `replication_state` | server → all | unreliable | `[ { id, authority, prefabPath, slot?, x?, y?, z?, angle?, sprite? } ]` |
| `replication_despawn` | server → all | reliable | `{ id }` |
| `player_input` | client → server | unreliable | `{ id, input: { up, down, left, right, shoot } }` |
| `entity_rpc` | both ways | reliable / unreliable | `{ entityId, method, data }` |

All of these except `your_player_id` are JSON payloads with the 2-byte id header.

---

## 9. The `Net` Lua API

The scripting bridge (`Engine/src/Scripting/LuaAPI/Plugins/MultiplayerAPI.cpp`) exposes the `Net`
table. Most games only need the first three groups.

### Identity & role

| Call | Returns |
|---|---|
| `Net.isNetworked()` | false in single-player, true when a host exists |
| `Net.isServer()` | true on the server |
| `Net.localId()` | this client's network id, or `nil` |
| `Net.localEntity()` | this client's player entity, or `nil` |
| `Net.networkId(entity)` | an entity's network id, or `nil` |
| `Net.entityFromId(id)` | the entity bound to a network id, or `nil` |
| `Net.isLocal(entity)` | true when the entity belongs to this client |
| `Net.playerSlot(entity)` / `Net.maxPlayers()` | color slot / the cap |

`Net.replicate(entity, authority?, syncTransform?, syncSprite?, prefabPath?)` marks a **runtime**
entity (a bullet, an effect) as replicated. On the server it assigns a network id immediately and the
replicator announces and syncs it; on a client it only tags the entity.

`Net.track(entity, id)` / `Net.untrack(entity)` are manual binding helpers for advanced cases — the
engine normally binds everything itself.

### Input

```lua
local input = Net.input(entity)  -- { up, down, left, right, shoot } or nil
```

Symmetric across roles (see §7.3 above).

### Custom messages (escape hatch)

When declarative state sync is not enough, the low-level API is still there:

```lua
Net.register("chat")                 -- returns/looks up a deterministic id
Net.on("chat", function(peer, data) ... end)
Net.send(peer, "chat", { text = "hi" }, true)   -- reliable
Net.broadcast("notice", { text = "go" })
Net.onConnect(function(peer) ... end)
Net.onDisconnect(function(peer) ... end)
```

### Entity RPC

For per-entity custom actions (an ability, a shop, a turn), use RPCs and keep the entity binding.
`Net.rpc` is a **function that takes the entity as its first argument** — there is no `entity:rpc(...)`
method:

```lua
-- sender
Net.rpc(entity, "spawnProjectile", { power = 2 })        -- reliable
Net.rpcUnreliable(entity, "aim", { angle = 0.5 })        -- high-frequency

-- receiver
Net.onEntityRpc("spawnProjectile", function(entity, data)
    log.info("spawn a projectile with power " .. tostring(data.power))
end)
```

An RPC carries the sender entity's network id; the engine resolves it back to the local entity
before calling the handler, so you never juggle raw ids. **Direction is fixed:** a **client's** RPC
goes to the **server only**; the **server's** RPC goes to **every client**. The sender entity must
already have a network id (see [`Net.replicate`](#identity-role)).

---

## 10. Worked example: two players shoot an enemy

```
t0  Server boots. Initialize(): the Player template is removed and its prefab path
    remembered; the Pata-Pata enemy gets networkId = its scene entity id;
    nextNetworkId = 1,000,000.

t1  Client A connects.               ENet CONNECT
      Server: create placeholder(id 1000000), send your_player_id, queue Connect.
    Client A: receives your_player_id → localNetworkId = 1000000.

t2  Server ProcessConnect:
      assign slot 0, spawn Player.prefab, destroy placeholder, SendSpawn{slot 0}.
    Client A: replication_spawn → slot applied, Player.prefab spawned, local entity bound.

t3  Client B connects → slot 1, id 1000001, same flow.

t4  Every tick the server broadcasts replication_state for both players and the
    enemy; each client draws three entities with the right colors.

t5  Client A holds RIGHT+SHOOT:
      client → player_input{ id:1000000, right:true, shoot:true }
      server stores it; player.lua reads Net.input → moves A 500 px/s;
      playerGun.lua sees the shoot edge → creates a bullet →
        Net.replicate(bullet, "Server", true, true)
      server assigns id 1000002, SendSpawn, then syncs its Position/Sprite.

t6  The bullet hits the enemy (server-side collision):
      the enemy explodes on the server, then queueDestroy.
      End of frame: FlushQueues destroys the enemy and the bullet.

t7  Next BroadcastState: ids 21 and 1000002 are no longer broadcast but were in
    m_announcedIds → SendDespawn(21), SendDespawn(1000002), reliably.

t8  Both clients untrack and destroy those replicas. World is consistent again.
```

---

## 11. Diagnostics

Two environment variables stay out of the way unless you ask for them:

| Variable | Effect |
|---|---|
| `RTYPE_NET_DEBUG=1` | once-per-second logs of server player positions, client local position, and both input streams |
| `RTYPE_TEST_INPUT=up\|down\|left\|right\|shoot` | forces a client input bit, so headless smoke tests can drive the full input → server → state path without a keyboard |

Useful log lines:

```
[Replication] server ready (player prefab: Prefabs/Player.prefab)
[Replication] client ready (player prefab: <none>)
[Replication] Spawned player entity 25 (network id 1000000, slot 0)
[Replication] Assigned network id 1000002 to entity 41
[Replication] Spawned prefab 'Prefabs/Player.prefab' entity=26 network id=1000001
[Replication] Despawned replica network id 21
```

---

## 12. Design notes & current limitations

These are deliberate trade-offs of the current phase, not bugs.

- **No client-side prediction.** A client renders the last server snapshot, so input has one tick of
  latency even on localhost, and remote players move at the snapshot rate. This is the single
  biggest "feel" item left. The architecture already supports it: fixed
  tick, full rewindable snapshots, ownership separated between simulate and interpolate.
- **No interpolation.** Snapshots are applied directly to `Position`/`Rotation`; remote motion can
  look steppy below the tick rate.
- **Full snapshots, not deltas.** Simple and rewindable; bandwidth grows with entity count. Fine for
  the current scale.
- **Single channel (0).** The API supports more for prioritisation, but nothing uses it yet.
- **Client collision still runs locally** on replicated entities, so a contact/wall jitter is
  possible on clients. Hits that matter are resolved on the server; client bullet replicas carry no
  collider.
- **No lobby, matchmaking, authentication or reconnect.** You connect to a host and port. The server
  trusts the input it receives.
- **Hard cap of 5 players**, enforced both by `PlayerSlotState::kMaxPlayers` and by ENet's
  `maxClients` (see [§14](#14-todos-and-planned-work)).
- **One player prefab per game.** Initialisation keeps a *single* player prefab path (the first
  `Client` template's `prefabPath`) and destroys **all** `Client` templates, so every connection
  spawns the same prefab. Different player classes are not expressible yet — vary per-slot
  appearance or behaviour instead.
- **No private messages from Lua.** `Net.broadcast` reaches every client, and Lua has no
  entity → peer lookup, so it cannot `Net.send` to one client. Hidden information (a hand, fog of
  war) cannot be withheld through the API — only by not sending it.

---

## 13. Extending it

- **Make an entity networked:** add a `NetworkComponent` (on a prefab or in the scene) — no other
  code. Pick `authority` and the sync flags.
- **Replicate a runtime entity:** call `Net.replicate(entity, ...)` from the script that creates it.
- **Sync a new field:** add it to the snapshot built in `BroadcastState` and read it back in
  `ApplyEntityState` (`Engine/src/Systems/ReplicationSystem.cpp`).
- **Add a packet type:** `packetRegistry.GetOrRegister("my_packet")`, then `router.On(id, handler)`;
  send with `CreateRawPacket` / `CreatePacket`.
- **Raise the player cap:** change `PlayerSlotState::kMaxPlayers` and the sprite-block mapping in
  your player script; the ENet `maxClients` follows automatically. (Making this configurable instead
  of compile-time is a [TODO](#14-todos-and-planned-work).)

---

## 14. TODOs and planned work

Open items, collected from the limitations above and from "editor & Lua only" authoring. The first
two are the ones that most often bite a game built without touching C++.

- [ ] **Prefab instantiation from Lua.** Prefabs are spawned only by C++ today
  (`PrefabSpawner` → `SpawnFromPrefab`): the server spawns the player on connect, and clients spawn
  the prefab named in a `replication_spawn` / `replication_state` message. A Lua script can only
  `Entity.create()` a bare entity and rebuild it by hand. Expose something like
  `Entity.instantiate("Prefabs/Tower.prefab", x, y)` so scripts can spawn fully-authored objects
  (with their own scripts and components) directly. This is the single biggest "editor & Lua only"
  limitation.
- [ ] **Configurable player cap (6+ players).** `PlayerSlotState::kMaxPlayers` is a compile-time `5`,
  and the server host is created with `maxClients = kMaxPlayers`, so ENet refuses the 6th connection
  before the game ever sees it. Make the cap a per-project setting (read from the scene / editor) and
  have the player script's color/sprite mapping follow it (`slot N → block N`). The editor's Client
  Count clamp already reads the constant, so it would follow the setting automatically.
- [ ] **Client-side prediction + reconciliation** for the local player (removes input latency).
- [ ] **Interpolation** of remote entities (smooths motion below the tick rate).
- [ ] **Editor network panel** — peers, per-entity authority, sync rates, packet log, and simulated
  latency / jitter / loss for testing.

See the [Editor Manual → Play Multiplayer](../editor/multiplayer.md), including
[recipes for different genres](../editor/multiplayer.md#7-recipes-for-different-genres), for how to
author and launch all of this from the editor.
