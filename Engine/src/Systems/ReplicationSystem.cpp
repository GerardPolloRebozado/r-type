#include "Systems/ReplicationSystem.hpp"

#include "Components/NetworkComponent.hpp"
#include "Components/Position.hpp"
#include "Components/Rotation.hpp"
#include "Components/SpriteComponent.hpp"
#include "ECS/Registry.hpp"
#include "Inputs/InputManager.hpp"
#include "Logging/Logger.hpp"
#include "Resources/NetworkHostResource.hpp"
#include "Resources/PacketRegistryResource.hpp"
#include "Resources/PacketRouterResource.hpp"
#include "Resources/PlayerInputState.hpp"
#include "Resources/PlayerSlotState.hpp"
#include "Resources/PrefabSpawner.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <glaze/glaze.hpp>
#include <string>
#include <unordered_set>

namespace Engine::Multiplayer {
ReplicationSystem::ReplicationSystem() {
    const char* debug = std::getenv("RTYPE_NET_DEBUG");
    m_debug = debug != nullptr && std::string(debug) != "0";
}

namespace {
constexpr Entity kInvalidEntity = static_cast<Entity>(-1);
/** Default speed for network entities that have no gameplay script to move them. */
constexpr float kDefaultMoveSpeed = 500.0f;

bool GenericBool(const glz::generic::object_t& obj, const char* key, bool fallback = false) {
    const auto it = obj.find(key);
    if (it == obj.end() || !it->second.is_boolean()) {
        return fallback;
    }
    return it->second.get<bool>();
}

double GenericNumber(const glz::generic::object_t& obj, const char* key, double fallback = 0.0) {
    const auto it = obj.find(key);
    if (it == obj.end() || !it->second.is_number()) {
        return fallback;
    }
    return it->second.get<double>();
}

std::string GenericString(const glz::generic::object_t& obj, const char* key,
                          const std::string& fallback = {}) {
    const auto it = obj.find(key);
    if (it == obj.end() || !it->second.is_string()) {
        return fallback;
    }
    return it->second.get_string();
}

std::set<Action> ParseNetInput(const glz::generic_json<>& value) {
    std::set<Action> input;
    if (glz::read_json(input, value)) {
        return {};
    }
    return input;
}

/** Adds/replaces NetworkComponent fields while keeping anything the prefab set. */
NetworkComponent ReadNetworkComponent(Registry& registry, Entity entity) {
    if (registry.HasComponent<NetworkComponent>(entity)) {
        return registry.GetComponent<NetworkComponent>(entity);
    }
    return NetworkComponent{};
}

void ApplySpriteState(Registry& registry, Entity entity, const glz::generic& value) {
    if (!value.is_object()) {
        return;
    }
    const auto& obj = value.get_object();
    if (!registry.HasComponent<SpriteComponent>(entity)) {
        registry.AddComponent<SpriteComponent>(entity, SpriteComponent{});
    }
    auto& sprite = registry.GetComponent<SpriteComponent>(entity);

    const std::string path = GenericString(obj, "path", sprite.assetPath);
    if (path != sprite.assetPath) {
        sprite.assetPath = path;
        // Drop the cached texture so SpriteSystem reloads the new path.
        sprite.sprite.reset();
    }
    if (obj.find("offsetX") != obj.end()) {
        sprite.offsetX = static_cast<float>(GenericNumber(obj, "offsetX", sprite.offsetX));
    }
    if (obj.find("offsetY") != obj.end()) {
        sprite.offsetY = static_cast<float>(GenericNumber(obj, "offsetY", sprite.offsetY));
    }
    if (obj.find("scaleX") != obj.end()) {
        sprite.scaleX = static_cast<float>(GenericNumber(obj, "scaleX", sprite.scaleX));
    }
    if (obj.find("scaleY") != obj.end()) {
        sprite.scaleY = static_cast<float>(GenericNumber(obj, "scaleY", sprite.scaleY));
    }
    if (obj.find("flipX") != obj.end()) {
        sprite.flipX = GenericBool(obj, "flipX", sprite.flipX);
    }
    if (obj.find("flipY") != obj.end()) {
        sprite.flipY = GenericBool(obj, "flipY", sprite.flipY);
    }
}

/** Applies one state snapshot entry to a local entity. */
void ApplyEntityState(Registry& registry, Entity entity, const glz::generic& state) {
    if (!state.is_object() || !registry.IsEntityAlive(entity)) {
        return;
    }
    const auto& obj = state.get_object();

    if (registry.HasComponent<Position>(entity)) {
        auto& position = registry.GetComponent<Position>(entity);
        if (obj.find("x") != obj.end()) {
            position.x = static_cast<float>(GenericNumber(obj, "x", position.x));
        }
        if (obj.find("y") != obj.end()) {
            position.y = static_cast<float>(GenericNumber(obj, "y", position.y));
        }
        if (obj.find("z") != obj.end()) {
            position.z = static_cast<int>(GenericNumber(obj, "z", position.z));
        }
    }
    if (registry.HasComponent<Rotation>(entity) && obj.find("angle") != obj.end()) {
        auto& rotation = registry.GetComponent<Rotation>(entity);
        rotation.angle = static_cast<float>(GenericNumber(obj, "angle", rotation.angle));
    }
    if (const auto it = obj.find("sprite"); it != obj.end()) {
        ApplySpriteState(registry, entity, it->second);
    }
}

/** Stores the color slot carried by a spawn/state message (client side). */
void ApplyReplicatedSlot(Registry& registry, std::size_t networkId,
                         const glz::generic::object_t& obj) {
    const auto it = obj.find("slot");
    if (it == obj.end() || !it->second.is_number()) {
        return;
    }
    if (!registry.HasResource<PlayerSlotState>()) {
        registry.AddResource<PlayerSlotState>();
    }
    registry.GetResource<PlayerSlotState>().byNetworkId[networkId] =
        static_cast<std::uint8_t>(it->second.get<double>());
}

glz::generic SerializeSpriteState(Registry& registry, Entity entity) {
    glz::generic out = glz::generic::object_t{};
    const auto& sprite = registry.GetComponent<SpriteComponent>(entity);
    auto& obj = out.get_object();
    obj["path"] = sprite.assetPath;
    obj["offsetX"] = sprite.offsetX;
    obj["offsetY"] = sprite.offsetY;
    obj["scaleX"] = sprite.scaleX;
    obj["scaleY"] = sprite.scaleY;
    obj["flipX"] = sprite.flipX;
    obj["flipY"] = sprite.flipY;
    return out;
}
} // namespace

void ReplicationSystem::Update(Registry& registry, float dt) {
    if (!registry.HasResource<NetworkHostResource>() ||
        !registry.HasResource<PacketRegistryResource>()) {
        return;
    }

    auto& host = registry.GetResource<NetworkHostResource>();
    auto& packetRegistry = registry.GetResource<PacketRegistryResource>();

    if (!m_initialized) {
        Initialize(registry, host);
        m_initialized = true;
    }

    // Register handlers before the first poll so a very fast client cannot send
    // input or receive a spawn message that nobody is listening for yet
    if (!m_handlersRegistered && registry.HasResource<PacketRouterResource>()) {
        RegisterStateHandler(registry);
        RegisterSpawnHandler(registry);
        RegisterDespawnHandler(registry);
        RegisterInputHandler(registry);
        m_handlersRegistered = true;
    }

    for (const auto& event : host.DrainEvents()) {
        if (event.type == NetworkHostResource::ConnectionEvent::Type::Connect) {
            ProcessConnect(registry, host, event.peer, event.entity, event.networkId);
        } else {
            ProcessDisconnect(registry, host, event.peer, event.entity, event.networkId);
        }
    }

    if (host.IsServer()) {
        // Scripted entities move themselves through Net.input(entity). Only
        // script-less network entities fall back to the engine default
        BroadcastState(registry, host, packetRegistry);
    } else if (host.GetLocalNetworkId().has_value()) {
        SendLocalInput(registry, host, packetRegistry);
        if (m_debug) {
            ++m_debugTicks;
            const auto localEntity = host.GetLocalEntity();
            if (m_debugTicks % 60 == 0 && localEntity.has_value() &&
                registry.IsEntityAlive(*localEntity) &&
                registry.HasComponent<Position>(*localEntity)) {
                const auto& position = registry.GetComponent<Position>(*localEntity);
                log_info("[NetDebug] client local entity=" + std::to_string(*localEntity) +
                         " x=" + std::to_string(position.x) + " y=" + std::to_string(position.y));
            }
        }
    }
}

void ReplicationSystem::OnSceneUnloaded(Registry& registry) {
    if (!m_initialized) {
        return;
    }
    log_error("[Replication] Scene switched during a network session. Scene changes are not "
              "synced between server and clients yet, so machines may now show different "
              "scenes.");
}

void ReplicationSystem::Initialize(Registry& registry, NetworkHostResource& host) {
    if (!registry.HasResource<PlayerInputState>()) {
        registry.AddResource<PlayerInputState>();
    }

    // Client-authority entities in the scene are per-player templates.
    // Remember the first prefab path, then remove the templates: the server
    // spawns one instance per connection, and clients receive those
    // instances through replication_spawn. This applies to both roles so a
    // client never keeps a duplicate "local" player next to the server one.
    std::vector<Entity> templates;
    for (std::size_t i = 0; i < registry.GetEntityCount(); ++i) {
        const Entity entity{i};
        if (!registry.IsEntityAlive(entity) || !registry.HasComponent<NetworkComponent>(entity)) {
            continue;
        }
        const auto& network = registry.GetComponent<NetworkComponent>(entity);
        if (network.authority == "Client") {
            if (m_playerPrefabPath.empty() && !network.prefabPath.empty()) {
                m_playerPrefabPath = network.prefabPath;
            }
            templates.push_back(entity);
        }
    }
    for (const Entity entity : templates) {
        if (registry.IsEntityAlive(entity)) {
            registry.DestroyEntity(entity);
        }
    }

    // Server-authority scene entities exist on every machine. Reusing the
    // scene entity id as the network id lets clients match their local copy
    // without a spawn round-trip (and keeps parent/child links intact).
    for (std::size_t i = 0; i < registry.GetEntityCount(); ++i) {
        const Entity entity{i};
        if (!registry.IsEntityAlive(entity) || !registry.HasComponent<NetworkComponent>(entity)) {
            continue;
        }
        auto& network = registry.GetComponent<NetworkComponent>(entity);
        if (network.authority == "Server" && network.networkId == 0) {
            network.networkId = static_cast<std::uint64_t>(entity);
            host.TrackNetworkEntity(registry, entity, entity);
        }
        if (host.IsServer() && network.authority == "Server") {
            // Clients already have this entity from the shared scene, so it is
            // already "known": no spawn packet is needed.
            m_announcedIds.insert(network.networkId);
        }
    }

    // Runtime spawns live above the scene id range.
    if (host.IsServer()) {
        host.SetNextNetworkId(1'000'000);
    }

    log_info("[Replication] " + std::string(host.IsServer() ? "server" : "client") +
             " ready (player prefab: " +
             (m_playerPrefabPath.empty() ? std::string("<none>") : m_playerPrefabPath) + ")");
}

void ReplicationSystem::ProcessConnect(Registry& registry, NetworkHostResource& host,
                                       ENetPeer* peer, Entity placeholder, std::size_t networkId) {
    if (!host.IsServer()) {
        return;
    }
    // A peer can connect and disconnect between two polls. The queued connect
    // event is still processed, but the peer is already gone: do not create a
    // player that nobody owns.
    if (peer == nullptr || peer->state != ENET_PEER_STATE_CONNECTED) {
        return;
    }
    if (networkId == 0) {
        networkId = host.GetNextNetworkId();
    }

    // Every connected player owns a distinct color slot. The transport already
    // caps connections at kMaxPlayers, so this only fails if a game raises that
    // cap without raising the slot count.
    if (!registry.HasResource<PlayerSlotState>()) {
        registry.AddResource<PlayerSlotState>();
    }
    const auto slot = registry.GetResource<PlayerSlotState>().Assign(networkId);
    if (!slot.has_value()) {
        log_error("[Replication] Refusing connection: all " +
                  std::to_string(PlayerSlotState::kMaxPlayers) + " player slots are taken");
        enet_peer_disconnect_later(peer, 0);
        return;
    }

    Entity entity = SpawnFromPrefab(registry, m_playerPrefabPath);
    if (entity == kInvalidEntity) {
        entity = (placeholder != kInvalidEntity && registry.IsEntityAlive(placeholder))
                     ? placeholder
                     : registry.CreateEntity();
    }

    // The placeholder exists only to carry the handshake and peer binding.
    if (placeholder != kInvalidEntity && placeholder != entity &&
        registry.IsEntityAlive(placeholder)) {
        host.UntrackNetworkEntity(registry, placeholder);
        registry.DestroyEntity(placeholder);
    }

    NetworkComponent network = ReadNetworkComponent(registry, entity);
    network.networkId = networkId;
    network.authority = "Client";
    network.syncTransform = true;
    network.syncSprite = true;
    if (network.prefabPath.empty()) {
        network.prefabPath = m_playerPrefabPath;
    }
    registry.AddComponent<NetworkComponent>(entity, network);

    host.TrackNetworkEntity(registry, entity, networkId);
    host.SetPeerEntity(peer, entity);

    auto& packetRegistry = registry.GetResource<PacketRegistryResource>();
    SendSpawn(registry, host, packetRegistry, entity, network, slot);

    log_info("[Replication] Spawned player entity " + std::to_string(entity) + " (network id " +
             std::to_string(networkId) + ", slot " + std::to_string(*slot) + ")");
}

void ReplicationSystem::ProcessDisconnect(Registry& registry, NetworkHostResource& host,
                                          ENetPeer* /*peer*/, Entity entity,
                                          std::size_t networkId) {
    // If Poll did not already destroy the placeholder/replica, do it here.
    if (entity != kInvalidEntity && registry.IsEntityAlive(entity)) {
        host.UntrackNetworkEntity(registry, entity);
        registry.DestroyEntity(entity);
    }

    if (!host.IsServer() || networkId == 0) {
        return;
    }

    // Drop the last input so a reused id (or a despawned entity) never keeps
    // moving from stale input.
    if (registry.HasResource<PlayerInputState>()) {
        registry.GetResource<PlayerInputState>().byNetworkId.erase(networkId);
    }
    // Free the color slot so the next player can take it.
    if (registry.HasResource<PlayerSlotState>()) {
        registry.GetResource<PlayerSlotState>().Release(networkId);
    }

    // Poll already removed the entity and its maps; the despawn diff would
    // catch it next tick, but sending it now keeps disconnects immediate.
    auto& packetRegistry = registry.GetResource<PacketRegistryResource>();
    SendDespawn(registry, host, packetRegistry, networkId);
    log_info("[Replication] Despawned player network id " + std::to_string(networkId));
}

void ReplicationSystem::RegisterStateHandler(Registry& registry) {
    auto& router = registry.GetResource<PacketRouterResource>();
    const auto packetId =
        registry.GetResource<PacketRegistryResource>().GetOrRegister("replication_state");

    router.On(packetId, [this, &registry](Registry& /*r*/, ENetPeer* /*peer*/,
                                          const uint8_t* payload, std::size_t size) {
        if (size == 0 || payload == nullptr) {
            return;
        }
        auto& host = registry.GetResource<NetworkHostResource>();
        if (host.IsServer()) {
            return; // Servers publish state, they never consume it.
        }

        const std::string raw{reinterpret_cast<const char*>(payload), size};
        glz::generic array{};
        if (glz::read_json(array, raw) || !array.is_array()) {
            return;
        }

        for (const auto& entry : array.get_array()) {
            if (!entry.is_object()) {
                continue;
            }
            const auto& obj = entry.get_object();
            if (obj.find("id") == obj.end()) {
                continue;
            }
            const auto networkId = static_cast<std::size_t>(GenericNumber(obj, "id"));
            ApplyReplicatedSlot(registry, networkId, obj);

            Entity entity = kInvalidEntity;
            if (const auto found = host.GetEntityByNetworkId(networkId); found.has_value()) {
                entity = *found;
            } else {
                // Late join / first state packet: instantiate the prefab when
                // the sender provided one, otherwise use a bare entity (runtime
                // entities such as bullets only need Position + Sprite).
                const std::string prefabPath = GenericString(obj, "prefabPath");
                entity = SpawnFromPrefab(registry, prefabPath);
                if (entity == kInvalidEntity) {
                    entity = registry.CreateEntity();
                }

                NetworkComponent network = ReadNetworkComponent(registry, entity);
                network.networkId = networkId;
                network.authority = GenericString(obj, "authority", "Server");
                network.prefabPath = prefabPath;
                network.syncTransform = true;
                network.syncSprite = true;
                registry.AddComponent<NetworkComponent>(entity, network);
                host.TrackNetworkEntity(registry, entity, networkId);
                log_info("[Replication] Spawned replica entity " + std::to_string(entity) +
                         " (network id " + std::to_string(networkId) + ")");
            }

            ApplyEntityState(registry, entity, entry);
        }
    });
}

void ReplicationSystem::RegisterSpawnHandler(Registry& registry) {
    auto& router = registry.GetResource<PacketRouterResource>();
    const auto packetId =
        registry.GetResource<PacketRegistryResource>().GetOrRegister("replication_spawn");

    router.On(packetId, [this, &registry](Registry& /*r*/, ENetPeer* /*peer*/,
                                          const uint8_t* payload, std::size_t size) {
        if (size == 0 || payload == nullptr) {
            return;
        }
        auto& host = registry.GetResource<NetworkHostResource>();
        if (host.IsServer()) {
            return;
        }

        const std::string raw{reinterpret_cast<const char*>(payload), size};
        glz::generic message{};
        if (glz::read_json(message, raw) || !message.is_object()) {
            return;
        }
        const auto& obj = message.get_object();
        if (obj.find("id") == obj.end()) {
            return;
        }
        const auto networkId = static_cast<std::size_t>(GenericNumber(obj, "id"));
        if (host.GetEntityByNetworkId(networkId).has_value()) {
            return; // Already spawned by an earlier state snapshot.
        }

        // Remember the color slot before the prefab runs its script onCreate,
        // so player.lua already picks the right sprite block.
        ApplyReplicatedSlot(registry, networkId, obj);

        const std::string prefabPath = GenericString(obj, "prefabPath");
        Entity entity = SpawnFromPrefab(registry, prefabPath);
        if (entity == kInvalidEntity) {
            entity = registry.CreateEntity();
        }

        NetworkComponent network = ReadNetworkComponent(registry, entity);
        network.networkId = networkId;
        network.authority = GenericString(obj, "authority", "Server");
        network.prefabPath = prefabPath;
        network.syncTransform = true;
        network.syncSprite = true;
        registry.AddComponent<NetworkComponent>(entity, network);
        host.TrackNetworkEntity(registry, entity, networkId);

        log_info("[Replication] Spawned " +
                 std::string(prefabPath.empty() ? "entity" : "prefab '" + prefabPath + "'") +
                 " entity=" + std::to_string(entity) + " network id=" + std::to_string(networkId));
    });
}

void ReplicationSystem::RegisterDespawnHandler(Registry& registry) {
    auto& router = registry.GetResource<PacketRouterResource>();
    const auto packetId =
        registry.GetResource<PacketRegistryResource>().GetOrRegister("replication_despawn");

    router.On(packetId, [this, &registry](Registry& /*r*/, ENetPeer* /*peer*/,
                                          const uint8_t* payload, std::size_t size) {
        if (size == 0 || payload == nullptr) {
            return;
        }
        auto& host = registry.GetResource<NetworkHostResource>();
        if (host.IsServer()) {
            return;
        }

        const std::string raw{reinterpret_cast<const char*>(payload), size};
        glz::generic message{};
        if (glz::read_json(message, raw) || !message.is_object()) {
            return;
        }
        const auto& obj = message.get_object();
        if (obj.find("id") == obj.end()) {
            return;
        }
        const auto networkId = static_cast<std::size_t>(GenericNumber(obj, "id"));
        const auto found = host.GetEntityByNetworkId(networkId);
        if (!found.has_value()) {
            return;
        }
        const Entity entity = *found;
        host.UntrackNetworkEntity(registry, entity);
        if (registry.IsEntityAlive(entity)) {
            registry.DestroyEntity(entity);
        }
        log_info("[Replication] Despawned replica network id " + std::to_string(networkId));
    });
}

void ReplicationSystem::RegisterInputHandler(Registry& registry) {
    auto& router = registry.GetResource<PacketRouterResource>();
    const auto packetId =
        registry.GetResource<PacketRegistryResource>().GetOrRegister("player_input");

    router.On(packetId, [this, &registry](Registry& /*r*/, ENetPeer* peer, const uint8_t* payload,
                                          const std::size_t size) {
        if (size == 0 || payload == nullptr) {
            return;
        }
        auto& host = registry.GetResource<NetworkHostResource>();
        if (!host.IsServer()) {
            return;
        }

        const std::string raw{reinterpret_cast<const char*>(payload), size};
        glz::generic message{};
        if (glz::read_beve(message, raw) || !message.is_object()) {
            return;
        }
        const auto& obj = message.get_object();
        if (obj.find("id") == obj.end()) {
            return;
        }
        const auto networkId = static_cast<std::size_t>(GenericNumber(obj, "id"));
        const auto inputIt = obj.find("input");
        if (inputIt == obj.end()) {
            return;
        }

        // Store by network id, not entity: ProcessConnect may replace the
        // placeholder entity after the first input packet arrives.
        if (!registry.HasResource<PlayerInputState>()) {
            registry.AddResource<PlayerInputState>();
        }
        const auto parsed = ParseNetInput(inputIt->second);
        registry.GetResource<PlayerInputState>().byNetworkId[networkId] = parsed;
    });
}

void ReplicationSystem::BroadcastState(Registry& registry, NetworkHostResource& host,
                                       PacketRegistryResource& packetRegistry) {
    const auto packetId = packetRegistry.GetOrRegister("replication_state");
    ++m_debugTicks;
    const bool logThisTick = m_debug && (m_debugTicks % 60 == 0);

    glz::generic array = glz::generic::array_t{};
    std::vector<std::size_t> currentIds;
    currentIds.reserve(registry.GetEntityCount());

    for (std::size_t i = 0; i < registry.GetEntityCount(); ++i) {
        const Entity entity{i};
        if (!registry.IsEntityAlive(entity) || !registry.HasComponent<NetworkComponent>(entity)) {
            continue;
        }
        auto& network = registry.GetComponent<NetworkComponent>(entity);
        if (network.authority == "Shared") {
            continue;
        }

        // Runtime entity created by a script (bullet, effect, ...). Give it an
        // id and announce the spawn before the first state snapshot.
        if (network.networkId == 0) {
            network.networkId = host.GetNextNetworkId();
            host.TrackNetworkEntity(registry, entity, network.networkId);
            log_info("[Replication] Assigned network id " + std::to_string(network.networkId) +
                     " to entity " + std::to_string(entity));
        }
        if (!m_announcedIds.contains(network.networkId)) {
            SendSpawn(registry, host, packetRegistry, entity, network,
                      registry.HasResource<PlayerSlotState>()
                          ? registry.GetResource<PlayerSlotState>().Get(network.networkId)
                          : std::nullopt);
        }

        currentIds.push_back(network.networkId);

        glz::generic snapshot = glz::generic::object_t{};
        auto& obj = snapshot.get_object();
        obj["id"] = static_cast<double>(network.networkId);
        obj["authority"] = network.authority;
        obj["prefabPath"] = network.prefabPath;
        if (registry.HasResource<PlayerSlotState>()) {
            if (const auto slot = registry.GetResource<PlayerSlotState>().Get(network.networkId);
                slot.has_value()) {
                obj["slot"] = static_cast<double>(*slot);
            }
        }

        if (network.syncTransform && registry.HasComponent<Position>(entity)) {
            const auto& position = registry.GetComponent<Position>(entity);
            obj["x"] = position.x;
            obj["y"] = position.y;
            obj["z"] = static_cast<double>(position.z);
        }
        if (network.syncTransform && registry.HasComponent<Rotation>(entity)) {
            obj["angle"] = registry.GetComponent<Rotation>(entity).angle;
        }
        if (network.syncSprite && registry.HasComponent<SpriteComponent>(entity)) {
            obj["sprite"] = SerializeSpriteState(registry, entity);
        }
        if (logThisTick && network.authority == "Client" &&
            registry.HasComponent<Position>(entity)) {
            const auto& position = registry.GetComponent<Position>(entity);
            log_info("[NetDebug] server player id=" + std::to_string(network.networkId) +
                     " x=" + std::to_string(position.x) + " y=" + std::to_string(position.y));
        }

        array.get_array().push_back(std::move(snapshot));
    }

    // Anything announced but no longer broadcast has been destroyed (bullet
    // expiry, enemy death, disconnect). Tell clients reliably. Diffing against
    // the announced set (not just last tick's snapshot) also catches entities
    // created and destroyed between two broadcasts.
    const std::unordered_set<std::size_t> current(currentIds.begin(), currentIds.end());
    std::vector<std::size_t> gone;
    gone.reserve(m_announcedIds.size());
    for (const std::size_t announcedId : m_announcedIds) {
        if (!current.contains(announcedId)) {
            gone.push_back(announcedId);
        }
    }
    for (const std::size_t goneId : gone) {
        SendDespawn(registry, host, packetRegistry, goneId);
    }

    if (array.get_array().empty()) {
        return;
    }

    std::string json;
    if (glz::write_json(array, json)) {
        log_error("[Replication] Failed to serialize state snapshot");
        return;
    }

    auto* packet = NetworkHostResource::CreateRawPacket(
        static_cast<std::uint16_t>(packetId), reinterpret_cast<const uint8_t*>(json.data()),
        json.size(), false);
    if (packet != nullptr) {
        host.SendAllHost(packet, 0);
    }
}

void ReplicationSystem::SendSpawn(Registry& registry, NetworkHostResource& host,
                                  PacketRegistryResource& packetRegistry, Entity entity,
                                  const NetworkComponent& network,
                                  std::optional<std::uint8_t> slot) {
    (void)registry;
    (void)entity;
    if (!host.IsServer()) {
        return;
    }
    glz::generic message = glz::generic::object_t{};
    auto& obj = message.get_object();
    obj["id"] = static_cast<double>(network.networkId);
    obj["authority"] = network.authority;
    obj["prefabPath"] = network.prefabPath;
    if (slot.has_value()) {
        obj["slot"] = static_cast<double>(*slot);
    }

    std::string json;
    if (glz::write_json(message, json)) {
        return;
    }
    const auto packetId = packetRegistry.GetOrRegister("replication_spawn");
    auto* packet = NetworkHostResource::CreateRawPacket(
        static_cast<std::uint16_t>(packetId), reinterpret_cast<const uint8_t*>(json.data()),
        json.size(), true);
    if (packet != nullptr) {
        host.SendAllHost(packet, 0);
        m_announcedIds.insert(network.networkId);
    }
}

void ReplicationSystem::SendDespawn(Registry& registry, NetworkHostResource& host,
                                    PacketRegistryResource& packetRegistry, std::size_t networkId) {
    (void)registry;
    if (!host.IsServer()) {
        return;
    }
    glz::generic message = glz::generic::object_t{};
    message.get_object()["id"] = static_cast<double>(networkId);

    std::string json;
    if (glz::write_json(message, json)) {
        return;
    }
    const auto packetId = packetRegistry.GetOrRegister("replication_despawn");
    auto* packet = NetworkHostResource::CreateRawPacket(
        static_cast<std::uint16_t>(packetId), reinterpret_cast<const uint8_t*>(json.data()),
        json.size(), true);
    if (packet != nullptr) {
        host.SendAllHost(packet, 0);
        m_announcedIds.erase(networkId);
    }
}

void ReplicationSystem::SendLocalInput(Registry& registry, NetworkHostResource& host,
                                       PacketRegistryResource& packetRegistry) {
    const auto localNetworkId = host.GetLocalNetworkId();
    if (!localNetworkId.has_value() || !registry.HasResource<InputManager>()) {
        return;
    }
    auto* controller = registry.GetResource<InputManager>().GetController(1);
    if (controller == nullptr) {
        return;
    }

    glz::generic message = glz::generic::object_t{};
    auto& obj = message.get_object();
    obj["id"] = static_cast<double>(*localNetworkId);
    obj["input"] = controller->getNetworkedActiveActions();
    std::vector<std::byte> buffer{};

    if (glz::write_beve(message, buffer)) {
        return;
    }
    const auto packetId = packetRegistry.GetOrRegister("player_input");
    auto* packet = NetworkHostResource::CreateRawPacket(
        packetId, reinterpret_cast<const uint8_t*>(buffer.data()), buffer.size(), false);
    if (packet != nullptr) {
        host.Send(packet, 0);
    }
}

Entity ReplicationSystem::SpawnFromPrefab(Registry& registry, const std::string& prefabPath) {
    if (prefabPath.empty() || !registry.HasResource<PrefabSpawner>()) {
        return kInvalidEntity;
    }
    auto& spawner = registry.GetResource<PrefabSpawner>();
    if (!spawner.Spawn) {
        return kInvalidEntity;
    }
    return spawner.Spawn(registry, prefabPath);
}
} // namespace Engine::Multiplayer