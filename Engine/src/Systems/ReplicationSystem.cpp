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
#include "Resources/SceneManager.hpp"
#include "Resources/ScriptVariableState.hpp"
#include "Scripting/ScriptVariableSync.hpp"

#include <algorithm>
#include <chrono>
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
        return registry.RequireComponent<NetworkComponent>(entity);
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
    auto& sprite = registry.RequireComponent<SpriteComponent>(entity);

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
void ApplyEntityState(Registry& registry, Entity entity, const glz::generic& state,
                      std::unordered_map<std::size_t, std::vector<NetSnapshot>>& snapshots,
                      std::size_t networkId, double now) {
    if (!state.is_object() || !registry.IsEntityAlive(entity)) {
        return;
    }
    const auto& obj = state.get_object();

    if (registry.HasComponent<Position>(entity)) {
        auto& position = registry.RequireComponent<Position>(entity);
        float tx = position.x;
        float ty = position.y;
        int tz = position.z;
        bool hasPos = false;
        if (obj.find("x") != obj.end()) {
            tx = static_cast<float>(GenericNumber(obj, "x", 0.0)) / 10.0f;
            hasPos = true;
        }
        if (obj.find("y") != obj.end()) {
            ty = static_cast<float>(GenericNumber(obj, "y", 0.0)) / 10.0f;
            hasPos = true;
        }
        if (obj.find("z") != obj.end()) {
            tz = static_cast<int>(GenericNumber(obj, "z", position.z));
        }
        float ta = 0.0f;
        bool hasAngle = false;
        if (registry.HasComponent<Rotation>(entity) && obj.find("angle") != obj.end()) {
            ta = static_cast<float>(GenericNumber(obj, "angle", 0.0)) / 10.0f;
            hasAngle = true;
        }
        if (hasPos || hasAngle) {
            auto& buf = snapshots[networkId];
            NetSnapshot snap;
            snap.time = now;
            snap.x = hasPos ? tx : position.x;
            snap.y = hasPos ? ty : position.y;
            snap.angle = hasAngle ? ta
                                  : (registry.HasComponent<Rotation>(entity)
                                         ? registry.RequireComponent<Rotation>(entity).angle
                                         : 0.0f);
            buf.push_back(snap);
            if (buf.size() > 4) {
                buf.erase(buf.begin());
            }
            // First sample: snap immediately.
            if (buf.size() == 1) {
                if (hasPos) {
                    position.x = tx;
                    position.y = ty;
                    position.z = tz;
                }
                if (hasAngle) {
                    registry.RequireComponent<Rotation>(entity).angle = ta;
                }
            }
        }
        if (obj.find("z") != obj.end()) {
            position.z = tz;
        }
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
    const auto& sprite = registry.RequireComponent<SpriteComponent>(entity);
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

    // First update after a scene switch on the server: tell clients first,
    // then respawn the players. Both are reliable and ordered on channel 0,
    // so clients switch before they receive the new player spawns.
    if (m_sceneChanged && host.IsServer()) {
        m_sceneChanged = false;
        SendSceneLoad(registry, host, nullptr, true);
        if (!registry.HasResource<PlayerSlotState>()) {
            registry.AddResource<PlayerSlotState>();
        }
        auto& slots = registry.GetResource<PlayerSlotState>();
        for (const auto& [peer, networkId] : m_playersToRespawn) {
            if (peer == nullptr || peer->state != ENET_PEER_STATE_CONNECTED) {
                continue;
            }
            auto slot = slots.Get(networkId);
            if (!slot.has_value()) {
                slot = slots.Assign(networkId);
            }
            if (slot.has_value()) {
                SpawnPlayer(registry, host, peer, kInvalidEntity, networkId, *slot);
            }
        }
        m_playersToRespawn.clear();
    }

    // Register handlers before the first poll so a very fast client cannot send
    // input or receive a spawn message that nobody is listening for yet
    if (!m_handlersRegistered && registry.HasResource<PacketRouterResource>()) {
        RegisterStateHandler(registry);
        RegisterSpawnHandler(registry);
        RegisterDespawnHandler(registry);
        RegisterInputHandler(registry);
        RegisterSceneLoadHandler(registry);
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
                const auto& position = registry.RequireComponent<Position>(*localEntity);
                log_info("[NetDebug] client local entity=" + std::to_string(*localEntity) +
                         " x=" + std::to_string(position.x) + " y=" + std::to_string(position.y));
            }
        }
    }
}

void ReplicationSystem::Interpolate(Registry& registry) {
    if (!registry.HasResource<NetworkHostResource>()) {
        return;
    }
    auto& host = registry.GetResource<NetworkHostResource>();
    if (host.IsServer()) {
        return;
    }

    const double now =
        std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();
    constexpr double kDelay = 0.1; // 100 ms interpolation delay

    for (auto& [networkId, buf] : m_snapshots) {
        if (buf.size() < 2) {
            continue;
        }
        const auto entityOpt = host.GetEntityByNetworkId(networkId);
        if (!entityOpt.has_value()) {
            continue;
        }
        const Entity entity = *entityOpt;
        if (!registry.IsEntityAlive(entity)) {
            continue;
        }

        const double t = now - kDelay;
        const NetSnapshot* a = &buf.front();
        const NetSnapshot* b = &buf.back();
        for (size_t i = 0; i + 1 < buf.size(); ++i) {
            if (buf[i].time <= t && t <= buf[i + 1].time) {
                a = &buf[i];
                b = &buf[i + 1];
                break;
            }
            if (t > buf.back().time) {
                a = &buf[buf.size() - 2];
                b = &buf.back();
            }
        }
        if (a == b) {
            continue;
        }
        const double span = b->time - a->time;
        float frac = (span > 0.0) ? static_cast<float>((t - a->time) / span) : 1.0f;
        if (frac < 0.0f) {
            frac = 0.0f;
        }
        if (frac > 1.0f) {
            frac = 1.0f;
        }

        if (registry.HasComponent<Position>(entity)) {
            auto& pos = registry.RequireComponent<Position>(entity);
            pos.x = a->x + (b->x - a->x) * frac;
            pos.y = a->y + (b->y - a->y) * frac;
        }
        if (registry.HasComponent<Rotation>(entity)) {
            auto& rot = registry.RequireComponent<Rotation>(entity);
            float diff = b->angle - a->angle;
            while (diff > 180.0f) {
                diff -= 360.0f;
            }
            while (diff < -180.0f) {
                diff += 360.0f;
            }
            rot.angle = a->angle + diff * frac;
        }
    }
}

void ReplicationSystem::OnSceneUnloaded(Registry& registry) {
    if (!m_initialized || !registry.HasResource<NetworkHostResource>()) {
        return; // single-player, or no session yet: nothing to sync
    }
    if (registry.HasResource<ScriptVariableState>()) {
        registry.GetResource<ScriptVariableState>().Reset();
    }
    auto& host = registry.GetResource<NetworkHostResource>();
    if (host.IsServer()) {
        // Players are destroyed with the scene. Remember who to respawn.
        m_playersToRespawn.clear();
        for (const auto& player : host.ConnectedPlayers(registry)) {
            m_playersToRespawn.emplace_back(player.peer, player.networkId);
        }
        m_sceneChanged = true;
    }
    // Entity IDs restart at 1 in the next scene: drop every binding to the old
    // entities. Initialize runs again on the next update for the new scene.
    host.ResetEntityBindings();
    host.InvalidateQueuedEventEntities();
    m_announcedIds.clear();
    m_initialized = false;
}

void ReplicationSystem::Initialize(Registry& registry, NetworkHostResource& host) {
    if (!registry.HasResource<PlayerInputState>()) {
        registry.AddResource<PlayerInputState>();
    }
    if (!registry.HasResource<ScriptVariableState>()) {
        registry.AddResource<ScriptVariableState>();
    }
    // Each scene may define its own player template. Keep the previous one
    // when the new scene has none (e.g. a level reusing the lobby's player).
    const std::string previousPlayerPrefab = m_playerPrefabPath;
    m_playerPrefabPath.clear();

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
        const auto& network = registry.RequireComponent<NetworkComponent>(entity);
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
    if (m_playerPrefabPath.empty()) {
        m_playerPrefabPath = previousPlayerPrefab;
    }

    // Server-authority scene entities exist on every machine. Reusing the
    // scene entity id as the network id lets clients match their local copy
    // without a spawn round-trip (and keeps parent/child links intact).
    for (std::size_t i = 0; i < registry.GetEntityCount(); ++i) {
        const Entity entity{i};
        if (!registry.IsEntityAlive(entity) || !registry.HasComponent<NetworkComponent>(entity)) {
            continue;
        }
        auto& network = registry.RequireComponent<NetworkComponent>(entity);
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

    // Runtime spawns live above the scene id range. Never move the counter
    // back: after a scene switch, players and runtime entities from earlier
    // scenes still own ids in that range.
    if (host.IsServer() && host.PeekNextNetworkId() < 1'000'000) {
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

    // The client may have started on another scene: move it to ours before
    // it receives any spawn.
    SendSceneLoad(registry, host, peer, false);
    SpawnPlayer(registry, host, peer, placeholder, networkId, *slot);
}

void ReplicationSystem::SpawnPlayer(Registry& registry, NetworkHostResource& host, ENetPeer* peer,
                                    Entity placeholder, std::size_t networkId, std::uint8_t slot) {
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
    host.BindPeer(entity, peer);

    auto& packetRegistry = registry.GetResource<PacketRegistryResource>();
    SendSpawn(registry, host, packetRegistry, entity, network, slot);

    log_info("[Replication] Spawned player entity " + std::to_string(entity) + " (network id " +
             std::to_string(networkId) + ", slot " + std::to_string(slot) + ")");
}

void ReplicationSystem::SendSceneLoad(Registry& registry, NetworkHostResource& host, ENetPeer* peer,
                                      bool reload) {
    if (!host.IsServer() || !registry.HasResource<SceneManager>()) {
        return;
    }
    const std::string& scene = registry.GetResource<SceneManager>().Current();
    if (scene.empty()) {
        return;
    }
    glz::generic message = glz::generic::object_t{};
    auto& obj = message.get_object();
    obj["scene"] = scene;
    obj["reload"] = reload;
    Buffer buffer;
    if (glz::write_beve(message, buffer)) {
        return;
    }
    const auto packetId =
        registry.GetResource<PacketRegistryResource>().GetOrRegister("scene_load");
    auto* packet =
        NetworkHostResource::CreateRawPacket(static_cast<std::uint16_t>(packetId), buffer, true);
    if (packet == nullptr) {
        return;
    }
    if (peer != nullptr) {
        NetworkHostResource::Send(peer, packet, 0);
    } else {
        host.SendAllHost(packet, 0);
        log_info("[Replication] Told clients to load scene " + scene);
    }
}

void ReplicationSystem::RegisterSceneLoadHandler(Registry& registry) {
    auto& router = registry.GetResource<PacketRouterResource>();
    const auto packetId =
        registry.GetResource<PacketRegistryResource>().GetOrRegister("scene_load");

    router.On(packetId, [&registry](Registry& /*r*/, ENetPeer* /*peer*/, const uint8_t* payload,
                                    std::size_t size) {
        if (size == 0 || payload == nullptr) {
            return;
        }
        if (registry.GetResource<NetworkHostResource>().IsServer()) {
            return; // only clients follow the server's scene
        }
        const std::string raw{reinterpret_cast<const char*>(payload), size};
        glz::generic message{};
        if (glz::read_beve(message, raw) || !message.is_object()) {
            return;
        }
        const auto& obj = message.get_object();
        const std::string scene = GenericString(obj, "scene");
        if (scene.empty()) {
            return;
        }
        if (!registry.HasResource<SceneManager>()) {
            log_error("[Replication] Server switched to scene " + scene +
                      " but this game has no SceneManager to follow it.");
            return;
        }
        auto& scenes = registry.GetResource<SceneManager>();
        if (!GenericBool(obj, "reload") && scenes.Current() == scene) {
            return; // already on the server's scene (normal connect)
        }
        log_info("[Replication] Server switched to scene " + scene);
        // Applied at the end of the frame. Until then Poll leaves the next
        // packets (the new scene's spawns) queued.
        scenes.Load(scene);
    });
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
    if (registry.HasResource<ScriptVariableState>()) {
        auto& variables = registry.GetResource<ScriptVariableState>();
        if (entity != kInvalidEntity) {
            variables.Remove(entity);
        }
        variables.RemovePending(networkId);
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
        if (glz::read_beve(array, raw) || !array.is_array()) {
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

            const double now =
                std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch())
                    .count();
            ApplyEntityState(registry, entity, entry, m_snapshots, networkId, now);
            if (const auto vars = obj.find("vars"); vars != obj.end() && vars->second.is_object()) {
                ApplyReplicatedScriptVariables(registry, networkId, entity,
                                               vars->second.get_object());
            }
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
        if (glz::read_beve(message, raw) || !message.is_object()) {
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
        if (glz::read_beve(message, raw) || !message.is_object()) {
            return;
        }
        const auto& obj = message.get_object();
        if (obj.find("id") == obj.end()) {
            return;
        }
        const auto networkId = static_cast<std::size_t>(GenericNumber(obj, "id"));
        const auto found = host.GetEntityByNetworkId(networkId);
        if (!found.has_value()) {
            if (registry.HasResource<ScriptVariableState>()) {
                registry.GetResource<ScriptVariableState>().RemovePending(networkId);
            }
            return;
        }
        const Entity entity = *found;
        if (registry.HasResource<ScriptVariableState>()) {
            auto& variables = registry.GetResource<ScriptVariableState>();
            variables.Remove(entity);
            variables.RemovePending(networkId);
        }
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

    if (!registry.IsComponentRegistered<NetworkComponent>()) {
        return;
    }
    auto netArray = registry.GetComponentArray<NetworkComponent>();

    std::vector<std::size_t> currentIds;
    currentIds.reserve(netArray->size());

    for (const auto& [entity, networkRef] : *netArray) {
        if (!registry.IsEntityAlive(entity)) {
            continue;
        }
        auto& network = registry.RequireComponent<NetworkComponent>(entity);
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
            const auto& position = registry.RequireComponent<Position>(entity);
            obj["x"] = static_cast<int>(std::lround(position.x * 10.0f));
            obj["y"] = static_cast<int>(std::lround(position.y * 10.0f));
            obj["z"] = position.z;
        }
        if (network.syncTransform && registry.HasComponent<Rotation>(entity)) {
            obj["angle"] = static_cast<int>(
                std::lround(registry.RequireComponent<Rotation>(entity).angle * 10.0f));
        }
        if (network.syncSprite && registry.HasComponent<SpriteComponent>(entity)) {
            obj["sprite"] = SerializeSpriteState(registry, entity);
        }
        if (registry.HasResource<ScriptVariableState>()) {
            auto& variables = registry.GetResource<ScriptVariableState>();
            if (const auto* registration = variables.Find(entity); registration != nullptr) {
                glz::generic vars = glz::generic::object_t{};
                auto& varsObj = vars.get_object();
                if (ReadScriptVariables(registry, entity, registration->fields, varsObj) &&
                    !varsObj.empty()) {
                    obj["vars"] = std::move(vars);
                }
            }
        }
        if (logThisTick && network.authority == "Client" &&
            registry.HasComponent<Position>(entity)) {
            const auto& position = registry.RequireComponent<Position>(entity);
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

    // Rate-limit state snapshots to ~20 Hz; spawns/despawns above are
    // still sent every tick (reliable, immediate).
    if (m_debugTicks % 3 != 0) {
        return;
    }

    Buffer buffer;
    if (glz::write_beve(array, buffer)) {
        log_error("[Replication] Failed to serialize state snapshot");
        return;
    }

    auto* packet = NetworkHostResource::CreateRawPacket(packetId, buffer, false);
    if (packet != nullptr) {
        host.SendAllHost(packet, 0);
    }
}

void ReplicationSystem::SendSpawn(const Registry& registry, NetworkHostResource& host,
                                  PacketRegistryResource& packetRegistry, Entity entity,
                                  const NetworkComponent& network,
                                  const std::optional<std::uint8_t> slot) {
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

    Buffer buffer;
    if (glz::write_beve(message, buffer)) {
        return;
    }
    const auto packetId = packetRegistry.GetOrRegister("replication_spawn");
    auto* packet = NetworkHostResource::CreateRawPacket(packetId, buffer, true);
    if (packet != nullptr) {
        host.SendAllHost(packet, 1);
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

    Buffer buffer;
    if (glz::write_beve(message, buffer)) {
        return;
    }
    const auto packetId = packetRegistry.GetOrRegister("replication_despawn");
    auto* packet = NetworkHostResource::CreateRawPacket(packetId, buffer, true);
    if (packet != nullptr) {
        host.SendAllHost(packet, 1);
    } else {
        log_error("[Replication] Failed to allocate despawn packet for network id " +
                  std::to_string(networkId));
    }
    m_announcedIds.erase(networkId);
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