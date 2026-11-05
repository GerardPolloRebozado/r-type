#ifndef ENGINE_SYSTEMS_REPLICATIONSYSTEM_HPP_
#define ENGINE_SYSTEMS_REPLICATIONSYSTEM_HPP_

#include "Components/NetworkComponent.hpp"
#include "ECS/Entity.hpp"
#include "ECS/Registry.hpp"
#include "ECS/System.hpp"
#include "Resources/PacketRegistryResource.hpp"
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "Compat/EnetWrapper.hpp"
#include <glaze/glaze.hpp>

namespace Engine::Multiplayer {

class NetworkHostResource;

/**
 * @class ReplicationSystem
 * @brief Declarative networking foundation.
 *
 * Reads @ref NetworkComponent on entities and drives spawn/despawn, identity
 * binding, and state sync — without gameplay scripts hand-rolling packets.
 *
 * Scene entities:
 *   - "Server": already present on every machine; the scene entity id doubles
 *     as the network id, so clients match their local copy without spawning.
 *   - "Client": a per-player template. The server destroys it and spawns one
 *     instance of NetworkComponent::prefabPath per connected client. Clients
 *     receive those instances through replication_spawn.
 *   - "Shared": ignored, built from the same scene on every machine.
 *
 * Scene switches (SceneManager): only the server switches. Its new scene is
 * sent to every client in a reliable "scene_load" message; clients load the
 * same scene, then receive the players again (same network ids and color
 * slots). A client that connects is told the server's current scene too, so
 * it never plays a different scene than the server.
 *
 * Runtime entities (bullets, effects): a script calls Net.replicate(entity,
 * ...) and the server assigns an id, broadcasts a spawn, then syncs state.
 * Clients instantiate a bare entity (or a prefab when prefabPath is set) and
 * apply Position/Rotation/Sprite from the snapshot.
 *
 * Design notes for future prediction/interpolation:
 *   - Replication runs on the fixed simulation tick.
 *   - State is published as full snapshots (rewindable), not deltas.
 *   - Ownership cleanly separates simulate vs interpolate entities.
 *   - Input is stored server-side and exposed to gameplay scripts through
 *     Net.input(entity), so movement stays in Lua without scripts ever touching
 *     ENet.
 */
/**
 * @struct NetSnapshot
 * @brief One timestamped transform sample received from the server.
 */
struct NetSnapshot {
    double time = 0.0;
    float x = 0.0f;
    float y = 0.0f;
    float angle = 0.0f;
};

struct PredictedInput {
    std::uint32_t seq = 0;
    float x = 0.0f;
    float y = 0.0f;
    float angle = 0.0f;
};

class ReplicationSystem : public System {
  public:
    ReplicationSystem();
    ~ReplicationSystem() noexcept override = default;

    void Update(Registry& registry, float dt) override;

    /**
     * @brief Prepares for the next scene: forgets entity bindings (IDs restart
     * at 1) and, on the server, remembers the connected players so they are
     * respawned and clients are told about the switch on the next update.
     */
    void OnSceneUnloaded(Registry& registry) override;

    void Interpolate(Registry& registry);

  private:
    void Initialize(Registry& registry, NetworkHostResource& host);
    void ProcessConnect(Registry& registry, NetworkHostResource& host, ENetPeer* peer,
                        Entity placeholder, std::size_t networkId);
    /** Spawns (or respawns) the player entity of a connected peer and announces it. */
    void SpawnPlayer(Registry& registry, NetworkHostResource& host, ENetPeer* peer,
                     Entity placeholder, std::size_t networkId, std::uint8_t slot);
    /**
     * Tells clients which scene the server is running. @p peer null means all
     * clients. With @p reload, clients reload even if already on that scene.
     */
    void SendSceneLoad(Registry& registry, NetworkHostResource& host, ENetPeer* peer, bool reload);
    void RegisterSceneLoadHandler(Registry& registry);
    void ProcessDisconnect(Registry& registry, NetworkHostResource& host, ENetPeer* peer,
                           Entity entity, std::size_t networkId);
    void BroadcastState(Registry& registry, NetworkHostResource& host,
                        PacketRegistryResource& packetRegistry);
    void ReconcileLocalPlayer(Registry& registry, Entity entity, const glz::generic::object_t& obj);
    void SendSpawn(const Registry& registry, NetworkHostResource& host,
                   PacketRegistryResource& packetRegistry, Entity entity,
                   const NetworkComponent& network,
                   std::optional<std::uint8_t> slot = std::nullopt);
    void SendDespawn(Registry& registry, NetworkHostResource& host,
                     PacketRegistryResource& packetRegistry, std::size_t networkId);
    void RegisterStateHandler(Registry& registry);
    void RegisterSpawnHandler(Registry& registry);
    void RegisterDespawnHandler(Registry& registry);
    void RegisterInputHandler(Registry& registry);
    void SendLocalInput(Registry& registry, NetworkHostResource& host,
                        PacketRegistryResource& packetRegistry);
    Entity SpawnFromPrefab(Registry& registry, const std::string& prefabPath);

    bool m_initialized = false;
    bool m_handlersRegistered = false;
    bool m_inSceneUnload = false;
    /** Server: the scene just changed; announce it and respawn players. */
    bool m_sceneChanged = false;
    /** Server: players to respawn in the new scene (peer + network id). */
    std::vector<std::pair<ENetPeer*, std::size_t>> m_playersToRespawn;
    /** Player prefab captured from the scene's Client template before it is removed. */
    std::string m_playerPrefabPath;
    /**
     * Ids already announced to clients with a reliable replication_spawn.
     * Scene Server entities are pre-seeded (both sides load the same scene);
     * runtime entities and players are added when their spawn is sent. Used to
     * avoid duplicate spawns and to detect destroyed entities that clients
     * still know about (including ones created and destroyed in one tick).
     */
    std::unordered_set<std::size_t> m_announcedIds;
    /** RTYPE_NET_DEBUG=1 prints player positions once per second. */
    bool m_debug = false;
    std::size_t m_debugTicks = 0;
    /** Separate counter for BroadcastState so client/server debug cadences don't interfere. */
    std::size_t m_broadcastTicks = 0;
    /** Client-side snapshot history for interpolation, keyed by network id. */
    std::unordered_map<std::size_t, std::vector<NetSnapshot>> m_snapshots;
    double m_serverTime = 0.0;
    double m_snapshotAccumulator = 0.0;
    double m_serverClockOffset = 0.0;
    bool m_hasServerClock = false;
    double m_interpDelay = 0.075;
    std::uint32_t m_inputSeq = 0;
    std::uint32_t m_lastAckedSeq = 0;
    std::vector<PredictedInput> m_predictionHistory;
};

} // namespace Engine::Multiplayer

#endif // ENGINE_SYSTEMS_REPLICATIONSYSTEM_HPP_
