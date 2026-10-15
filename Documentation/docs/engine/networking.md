# Networking

The engine provides a multiplayer SDK built on top of [ENet](http://enet.bespin.org/) and [Glaze](https://github.com/stephenberry/glaze). It is intentionally thin: you define your own packet structs and register handlers like if you where doing a REST API.

The reference implementation lives in `Games/MultiplayerLowLevelSDK/` (`PacketTypes.hpp`, `server.cpp`, `client.cpp`).

---

## Architecture Overview

```
[ ENetHost / ENetPeer (ENet) ]
              ↓
[ NetworkHostResource (Resource) ]  ──> Owns host, peers, networkId ↔ Entity mapping
              ↓
[ PacketRouterResource (Resource) ] ──> PacketTypeID → handler(Registry&, ENetPeer*, const T&)
              ↓
[ MultiplayerSystem (System) ]      ──> Calls NetworkHostResource::Poll(0, router, registry) every tick
              ↓
[ Your gameplay systems / handlers ]
```

4 pieces:

1. **`Engine::Multiplayer::ENetContext` (`Engine/include/ENetContext.hpp`):** RAII guard for ENet. Construct once in `main()`, keep alive until shutdown if your client is not connecting to the server, the isssue is probably because this has not been run.
2. **`Engine::Multiplayer::NetworkHostResource` (`Engine/include/Resources/NetworkHostResource.hpp`):** Host creation, `Connect` / `Send` / `SendAllHost`, packet factory, entity ↔ network-id tracking.
3. **`Engine::Multiplayer::PacketRouterResource` (`Engine/include/Resources/PacketRouterResource.hpp`):** Type-safe packet dispatch + `OnConnect` / `OnDisconnect` callbacks.
4. **`Engine::MultiplayerSystem` (`Engine/include/Systems/MultiplayerSystem.hpp`):** A normal `Engine::System` that polls the network every frame.

Wire format: `[uint16_t packetId little-endian][glaze BEVE payload]`. See `NetworkHostResource::CreatePacket()` and `PacketRouterResource::Dispatch()`.

---

## 1. Setup: Context, Host and System

Both client and server follow the same bootstrap:

```cpp
#include "ENetContext.hpp"
#include "Resources/NetworkHostResource.hpp"
#include "Resources/PacketRouterResource.hpp"
#include "Systems/MultiplayerSystem.hpp"

static void OnConnect(Engine::Registry& registry, ENetPeer* peer) { /* ... */ }
static void OnDisconnect(Engine::Registry& registry, ENetPeer* peer) { /* ... */ }

int main() {
    Engine::Multiplayer::ENetContext context; // enet_initialize() / enet_deinitialize()

    Engine::Application app(config);
    auto& registry = app.GetRegistry();

    // Server:
    registry.AddResource<Engine::Multiplayer::NetworkHostResource>(
        Engine::Multiplayer::NetworkHostResource::CreateServer(3000, 10));
    // Client:
    // registry.AddResource<Engine::Multiplayer::NetworkHostResource>(
    //     Engine::Multiplayer::NetworkHostResource::CreateClient(2));

    Engine::Multiplayer::NetworkHostResource::RegisterMultiplayerComponents(registry);

    auto& router = registry.AddResource<Engine::Multiplayer::PacketRouterResource>(
        OnConnect, OnDisconnect);

    registry.RegisterSystem<Engine::MultiplayerSystem>(); // Poll() every Update()
    app.Run();
}
```

Key APIs:

| Function | Server / Client | Description |
|---|---|---|
| `CreateServer(port, maxClients=32, channels=2)` | Server | Binds an `ENetHost` on `port`. `channels` separates e.g. game vs chat traffic. |
| `CreateClient(channels=2)` | Client | Creates an unbound host. Must call `Connect()` after. |
| `Connect(host, port, channels=2)` | Client | Returns `ENetPeer*` or `nullptr` on failure. Example: `host.Connect("127.0.0.1", 3000)`. The example uses `"192.168.202.217"`. |
| `RegisterMultiplayerComponents(registry)` | Both | Registers SDK-owned components (`PlayerInformation`, `YourPlayerId`). Idempotent; also called internally on server connect. |
| `IsServer()` | Both | `true` if created with `CreateServer()`. |

> `MultiplayerSystem::Update()` is just `networkHost.Poll(0, router, registry)` — non-blocking poll. It must be registered or no packets / connects / disconnects will fire.

---

## 2. Defining Packets

Packets are plain structs serialized with Glaze BEVE. Each distinct struct gets a unique `PacketTypeID` (`uint16_t`) via `GetPacketTypeID<T>()`. IDs `0`, `2`, `3` are reserved (`Ping`, `Connect`, `Disconnect`); user IDs start at `200` (`UserDefinedStart`).

From `Games/MultiplayerLowLevelSDK/PacketTypes.hpp`:

```cpp
#include "Components/Position.hpp"
#include "Resources/PacketRouterResource.hpp"

struct KeyPress {
    char key = 0;
};
const PacketTypeID KeyPressPacket = Engine::Multiplayer::GetPacketTypeID<KeyPress>();

struct NewPlayerPos {
    std::size_t id{};
    Engine::Position pos;
};
const PacketTypeID NewPlayerPosPacket = Engine::Multiplayer::GetPacketTypeID<NewPlayerPos>();
```

Notes:

* You can embed engine components directly (e.g. `Engine::Position`) — reflection in `Reflection/ComponentReflect.hpp` makes them serialize with zero manual code.
* Register custom structs as ECS components if you store them: `registry.RegisterComponent<KeyPress>();`.
* Two SDK packets already exist, no definition needed:

```cpp
struct PlayerInformation { std::size_t id = 0; }; // server-side tag, entity → network id
struct YourPlayerId { std::size_t id; };          // server → client: "this is your id"
const PacketTypeID YourPlayerIdPacket = Engine::Multiplayer::GetPacketTypeID<YourPlayerId>();
```

---

## 3. Sending Packets

Create with `NetworkHostResource::CreatePacket(id, data, reliable)`, then send:

```cpp
// Client → server (uses stored server_peer):
ENetPacket* packet = Engine::Multiplayer::NetworkHostResource::CreatePacket(
    KeyPressPacket, KeyPress{'d'}, true);
registry.GetResource<Engine::Multiplayer::NetworkHostResource>().Send(packet, 0);

// Server → one client:
Engine::Multiplayer::NetworkHostResource::Send(peer, packet, 0);

// Server → everyone:
registry.GetResource<Engine::Multiplayer::NetworkHostResource>()
    .SendAllHost(packet, 0);
```

* `reliable=true` maps to `ENET_PACKET_FLAG_RELIABLE`. Use it for joins, spawns, catch-up state. Use `false` for high-frequency inputs / positions (as the example does for `NewPlayerPos` broadcasts).
* `channel` (default `0`) selects the ENet channel. Use different channels to prioritize e.g. chat vs movement.
* ENet takes ownership of `ENetPacket*` once sent — do not free it yourself.

Example — client input system from `client.cpp`:

```cpp
class SendKeySystem : public Engine::System {
public:
    void Update(Engine::Registry& registry, float dt) override {
        KeyPress keyPressed;
        if (IsKeyDown(KEY_D)) keyPressed.key = 'd';
        if (IsKeyDown(KEY_A)) keyPressed.key = 'a';
        if (IsKeyDown(KEY_W)) keyPressed.key = 'w';
        if (IsKeyDown(KEY_S)) keyPressed.key = 's';
        if (keyPressed.key == 0) return;

        ENetPacket* packet = Engine::Multiplayer::NetworkHostResource::CreatePacket(
            KeyPressPacket, keyPressed, true);
        registry.GetResource<Engine::Multiplayer::NetworkHostResource>().Send(packet, 0);
    }
    void Render(Engine::Registry&) override {}
};
```

Example — server broadcast from `server.cpp`:

```cpp
const auto newPosition = NewPlayerPos{*networkId, position};
const auto* notify = Engine::Multiplayer::NetworkHostResource::CreatePacket(
    NewPlayerPosPacket, newPosition, false);
registry.GetResource<Engine::Multiplayer::NetworkHostResource>()
    .SendAllHost(notify, 0);
```

---

## 4. Receiving Packets: Router Handlers

Register a callback per packet ID. Signature: `void(Registry&, ENetPeer*, const T&)`.

```cpp
// Server: client input
router.RegisterHandler<KeyPress>(KeyPressPacket, KeyPressed);
// Client: spawn confirmation + world updates
router.RegisterHandler<Engine::Multiplayer::YourPlayerId>(
    Engine::Multiplayer::YourPlayerIdPacket, SpawnMyPlayer);
router.RegisterHandler<NewPlayerPos>(NewPlayerPosPacket, UpdatePlayerPosition);
```

Overload accepts lambdas, `std::function`, or plain function pointers.

Example — server authoritative movement (`server.cpp`):

```cpp
static void KeyPressed(Engine::Registry& registry, ENetPeer* peer, const KeyPress& keyPress) {
    const auto entity = Engine::Multiplayer::NetworkHostResource::GetEntityID(peer);
    if (!entity.has_value()) return;

    auto& host = registry.GetResource<Engine::Multiplayer::NetworkHostResource>();
    const auto networkId = host.GetNetworkId(registry, entity.value());
    if (!networkId.has_value()) return;

    auto& position = registry.GetComponent<Engine::Position>(entity.value());
    switch (keyPress.key) {
        case 'w': position.moved = true; position.y -= 1; break;
        case 'a': position.moved = true; position.x -= 1; break;
        case 's': position.moved = true; position.y += 1; break;
        case 'd': position.moved = true; position.x += 1; break;
        default: return;
    }
    const auto notify = Engine::Multiplayer::NetworkHostResource::CreatePacket(
        NewPlayerPosPacket, NewPlayerPos{*networkId, position}, false);
    registry.GetResource<Engine::Multiplayer::NetworkHostResource>()
        .SendAllHost(notify, 0);
}
```

Malformed BEVE payloads are silently dropped inside the router (`glz::read_beve` failure → no callback). Unknown packet IDs are ignored in `Dispatch()`.

---

## 5. Identity, Presence and Replication Helpers

The SDK maintains `networkId ↔ Entity ↔ ENetPeer` maps so you don't have to. Relevant for authoritative-server patterns like the example:

| Helper | Description |
|---|---|
| `GetEntityID(peer)` (static) | Server-side `Entity` auto-created for a peer on connect. `std::optional<Entity>`. |
| `SetPeerEntity(peer, entity)` (static) | Override the peer→entity binding. |
| `GetNetworkId(registry, entity)` | Server-assigned `std::size_t` id for an entity. |
| `GetEntityByNetworkId(id)` | Reverse lookup. Used by clients to find/create replicas. |
| `GetLocalNetworkId()` | Client-side id received via `YourPlayerId` packet. Empty on servers. |
| `GetLocalEntity()` | Client entity bound via `TrackNetworkEntity`. |
| `GetPeer(entity)` | Server-side peer for a player entity, if still connected. |
| `TrackNetworkEntity(registry, entity, networkId)` | Bind a locally spawned entity to a remote id + tag it with `PlayerInformation`. Use when spawning replicas. |

Example — client spawning / updating replicas (`client.cpp`):

```cpp
static void UpdatePlayerPosition(Engine::Registry& registry, const ENetPeer*,
                                 const NewPlayerPos& newPos) {
    auto& host = registry.GetResource<Engine::Multiplayer::NetworkHostResource>();
    auto playerEntity = host.GetEntityByNetworkId(newPos.id);
    if (!playerEntity.has_value()) {
        const auto entity = registry.CreateEntity();
        host.TrackNetworkEntity(registry, entity, newPos.id);
        registry.AddComponent(entity, Engine::ShapeComponent{.color = raylib::RED});
        playerEntity = entity;
    }
    registry.GetComponent<Engine::Position>(*playerEntity) = newPos.pos;
}

static void SpawnMyPlayer(Engine::Registry& registry, ENetPeer*,
                          const Engine::Multiplayer::YourPlayerId& player) {
    auto& host = registry.GetResource<Engine::Multiplayer::NetworkHostResource>();
    const auto playerEntity = registry.CreateEntity();
    registry.AddComponent(playerEntity, Engine::ShapeComponent{.color = raylib::WHITE});
    registry.AddComponent(playerEntity, Engine::Multiplayer::YourPlayerId{player.id});
    host.TrackNetworkEntity(registry, playerEntity, player.id);
    registry.RegisterSystem<SendKeySystem>(); // start sending input only after spawn
}
```

Example — server join / catch-up (`server.cpp` `OnConnect`):

```cpp
static void OnConnect(Engine::Registry& registry, ENetPeer* peer) {
    const auto& host = registry.GetResource<Engine::Multiplayer::NetworkHostResource>();
    const auto playerEntity = Engine::Multiplayer::NetworkHostResource::GetEntityID(peer);
    if (!playerEntity.has_value()) return;

    registry.AddComponent(*playerEntity, Engine::ShapeComponent{});
    const auto newPlayerId = host.GetNetworkId(registry, *playerEntity);
    if (!newPlayerId.has_value()) return;

    const auto& newPlayerPos = registry.GetComponent<Engine::Position>(*playerEntity);
    auto playerArray = registry.GetComponentArray<Engine::Multiplayer::PlayerInformation>();
    for (const auto& [existingEntity, existingPlayer] : *playerArray) {
        if (existingPlayer.id == *newPlayerId) continue;
        const auto existingPeer = host.GetPeer(existingEntity);
        if (!existingPeer.has_value()) continue;
        // 1. Send newcomer every existing player position (reliable catch-up)
        auto* catchUp = Engine::Multiplayer::NetworkHostResource::CreatePacket(
            NewPlayerPosPacket,
            NewPlayerPos{existingPlayer.id,
                         registry.GetComponent<Engine::Position>(existingEntity)}, true);
        Engine::Multiplayer::NetworkHostResource::Send(peer, catchUp, 0);
        // 2. Tell existing players about the newcomer
        auto* announce = Engine::Multiplayer::NetworkHostResource::CreatePacket(
            NewPlayerPosPacket, NewPlayerPos{*newPlayerId, newPlayerPos}, true);
        Engine::Multiplayer::NetworkHostResource::Send(*existingPeer, announce, 0);
    }
}
```

`OnConnect` / `OnDisconnect` are passed to the `PacketRouterResource` constructor and invoked by `Poll()` on ENet connect/disconnect events. The client example leaves them empty; all client spawn logic is driven by the `YourPlayerId` handler instead.

---

## 6. Full Minimal Flow (from the example)

**Server (`server.cpp`):**

1. `ENetContext context;`
2. `CreateServer(3000, 10)` + `RegisterMultiplayerComponents(registry)`.
3. `AddResource<PacketRouterResource>(OnConnect, OnDisconnect)`.
4. `router.RegisterHandler<KeyPress>(KeyPressPacket, KeyPressed);`
5. `RegisterSystem<MultiplayerSystem>();` → `app.Run();`

**Client (`client.cpp`):**

1. `ENetContext context;`
2. `CreateClient(2)` + `RegisterMultiplayerComponents(registry)` + `RegisterComponent<KeyPress>()`.
3. `host.Connect("192.168.202.217", 3000);`
4. `router.RegisterHandler<YourPlayerId>(YourPlayerIdPacket, SpawnMyPlayer);`
5. `router.RegisterHandler<NewPlayerPos>(NewPlayerPosPacket, UpdatePlayerPosition);`
6. `RegisterSystem<MultiplayerSystem>();` → `app.Run();` (`SendKeySystem` is registered later, inside `SpawnMyPlayer`).

Build targets (`Games/MultiplayerLowLevelSDK/CMakeLists.txt`): `Client` from `client.cpp`, `Server` from `server.cpp`, both linking `Engine`.

---

## 7. Limitations & Tips

* Low-level by design: no built-in interpolation, prediction, or interest management — `NewPlayerPos` is sent raw in the example.
* Server is authoritative in the example: clients only send `KeyPress`, the server moves `Engine::Position` and broadcasts. Keep this pattern to avoid cheating / desync.
* `Poll(timeoutMs, router, registry)` with `0` = non-blocking. Increase the timeout only for dedicated headless servers.
* Always check `std::optional` returns (`GetEntityID`, `GetNetworkId`, `GetPeer`, `GetEntityByNetworkId`) — peers may disconnect mid-frame.
* See also `Tests/PacketRoundTrip.cpp` for `CreatePacket` serialization round-trip expectations.
