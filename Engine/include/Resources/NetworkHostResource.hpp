/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** NetworkHost.hpp
*/
#ifndef G_ING_500_BAR_5_1_RTYPE_1_NETWORKHOST_HPP
#define G_ING_500_BAR_5_1_RTYPE_1_NETWORKHOST_HPP

#include "ECS/Registry.hpp"
#include "PacketRouterResource.hpp"

#include "Compat/EnetWrapper.hpp"
#include <cstddef>
#include <cstdint>
#include <glaze/glaze.hpp>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace Engine::Multiplayer {
class NetworkHostResource {
  public:
    /**
     *
     * @param port Where will this server be accesible
     * @param maxClients the maximum number of users that can connect at the same time
     * @param channels number of channels useful to separate the processing of messages like chat
     * and game so packet processing can be prioritized
     * @return a NetworkResource created as a server
     */
    static NetworkHostResource CreateServer(std::uint16_t port, std::size_t maxClients = 32,
                                            std::size_t channels = 2);

    /**
     *
     * @param channels number of channels useful to separate the processing of messages like chat
     * and game so packet processing can be prioritized
     * @return a NetworkResource created as a client
     */
    static NetworkHostResource CreateClient(size_t channels = 2);

    /**
     *
     * @param host address of the server we want to connect to
     * @param port port where the service of the server is running
     * @param channels number of channels useful to separate the processing of messages like chat
     * and game so packet processing can be prioritized
     * @return if able to connect an object representing where you connected, nullptr if the
     * connection did not succeed
     */
    ENetPeer* Connect(std::string_view host, uint16_t port, size_t channels = 2);

    /**
     * @brief Checks any new packets and sends the pending ones
     *
     * Stops early while a scene switch is pending (SceneManager), leaving the
     * rest of the packets queued: they belong to the next scene and are read
     * on the first poll after the switch.
     * @param timeoutMs time to wait till an event happens 0 to return inmediately
     * @param router router to use when a new packet is received
     * @param registry of the game
     */
    void Poll(uint32_t timeoutMs, const PacketRouterResource& router, Registry& registry);

    /**
     *
     * @param peer Peer that you want to know its entityID
     * @return Given a ENet peer it will return its entityID
     */
    static std::optional<Entity> GetEntityID(ENetPeer* peer);

    /**
     *
     * @param peer Where to save the entity ID
     * @param entity The entity to save in the peer
     */
    static void SetPeerEntity(ENetPeer* peer, Entity entity);

    /**
     * @brief Server-side: binds a player entity to its peer in both directions
     * (GetPeer(entity) and the entity stored on the peer).
     */
    void BindPeer(Entity entity, ENetPeer* peer) {
        m_entityToPeer[entity] = peer;
        SetPeerEntity(peer, entity);
    }

    bool IsServer() const {
        return m_isServer;
    }

    /**
     * @brief Assigns and returns the next network id.
     */
    std::size_t GetNextNetworkId() {
        return m_nextNetworkId++;
    }

    /**
     * @brief Sets the next id to assign.
     *
     * Replication reserves the low ids for entities that already exist in the
     * scene on every machine, and starts runtime spawns (players, bullets)
     * above them to avoid collisions.
     */
    void SetNextNetworkId(std::size_t id) {
        m_nextNetworkId = id;
    }

    /** @brief The next id GetNextNetworkId() will assign, without assigning it. */
    std::size_t PeekNextNetworkId() const {
        return m_nextNetworkId;
    }

    /**
     * @brief Network id assigned by the server to the given entity, if any.
     */
    std::optional<std::size_t> GetNetworkId(Registry& registry, Entity entity) const;

    /**
     * @brief Entity currently bound to a network id (server-owned players and
     * entities registered via TrackNetworkEntity), if any.
     */
    std::optional<Entity> GetEntityByNetworkId(std::size_t networkId) const;

    /**
     * @brief Network id the server assigned to this client (set automatically
     * when the YourPlayerId packet arrives). Empty on servers.
     */
    std::optional<std::size_t> GetLocalNetworkId() const {
        return m_localNetworkId;
    }

    /**
     * @brief Local player entity bound via TrackNetworkEntity. Empty until the
     * game spawns/binds it.
     */
    std::optional<Entity> GetLocalEntity() const {
        return m_localEntity;
    }

    /**
     * @brief Peer backing a server-side player entity, if still connected.
     */
    std::optional<ENetPeer*> GetPeer(Entity entity) const;

    struct ConnectionEvent {
        enum class Type { Connect, Disconnect };
        Type type;
        ENetPeer* peer;
        /** Player entity bound to the peer when the event was raised, if any. */
        Entity entity = static_cast<Entity>(-1);
        /** Network id bound to the peer when the event was raised, 0 if none. */
        std::size_t networkId = 0;
    };
    /** Queue a connect/disconnect for the replication system to process. */
    void QueueEvent(ConnectionEvent::Type type, ENetPeer* peer,
                    Entity entity = static_cast<Entity>(-1), std::size_t networkId = 0) {
        m_eventQueue.push_back(ConnectionEvent{type, peer, entity, networkId});
    }
    /** Drain queued connection events (called by ReplicationSystem each tick). */
    std::vector<ConnectionEvent> DrainEvents() {
        std::vector<ConnectionEvent> out = std::move(m_eventQueue);
        m_eventQueue.clear();
        return out;
    }

    /**
     * @brief Binds an entity to a network id and tags it with PlayerInformation.
     * Use this when spawning replicas of remote players (e.g. client spawning
     * an entity for NewPlayerPos.id)
     * @param registry game registry
     * @param entity entity to bind
     * @param networkId server-assigned id
     */
    void TrackNetworkEntity(Registry& registry, Entity entity, std::size_t networkId);

    /**
     * @brief Removes an entity's network binding (network id and peer). Call
     * this before destroying a replica so GetEntityByNetworkId no longer
     * resolves the stale entity.
     * @param registry game registry
     * @param entity entity to unbind
     */
    void UntrackNetworkEntity(Registry& registry, Entity entity);

    /** @brief A connected player on the server: its peer and network id. */
    struct ConnectedPlayer {
        ENetPeer* peer;
        std::size_t networkId;
    };

    /**
     * @brief Server-side: players with a spawned player entity whose peer is
     * still connected. Peers still waiting for their spawn (queued connect
     * event) are not included.
     */
    std::vector<ConnectedPlayer> ConnectedPlayers(Registry& registry) const;

    /**
     * @brief Forgets every entity binding (network id -> entity, entity ->
     * peer, local entity, and the entity stored on each peer).
     *
     * Called on a scene switch: entity ids restart at 1, so old bindings would
     * point at unrelated entities of the new scene. Network ids, the local
     * network id and connections are kept.
     */
    void ResetEntityBindings();

    /**
     * @brief Marks the entity of every queued connect/disconnect event as
     * invalid (the scene switch destroyed it). The network ids are kept.
     */
    void InvalidateQueuedEventEntities() {
        for (auto& event : m_eventQueue) {
            event.entity = static_cast<Entity>(-1);
        }
    }

    /**
     * @brief Sends a packet to all users
     * @param packet packet created, you can create one with NetworkHostResource::CreatePacket()
     * @param channel the channel to use to send the information
     */
    void SendAllHost(ENetPacket* packet, const uint8_t channel = 0) const {
        enet_host_broadcast(m_host, channel, packet);
    }

    /**
     *  @brief This function will send data to the specified peer.
     * @param peer where do we want to send it
     * @param channel the channel where the data would be send
     * @param packet
     */
    static void Send(ENetPeer* peer, ENetPacket* packet, const uint8_t channel = 0) {
        enet_peer_send(peer, channel, packet);
    }

    /**
     *  @brief This function will send any data to the specified peer
     * @param channel the channel where the data would be send
     * @param packet
     */
    void Send(ENetPacket* packet, const uint8_t channel = 0) const {
        Send(server_peer, packet, channel);
    }

    /**
     * Given a data struct it creates a packet to be send
     * Reliable should only be used outside of the game, for example if we are just sending the
     * player inputs we dont need to set it to reliable
     * @tparam TPacketId type of the packet to send
     * @tparam TData type of the data to send
     * @param id id of the type of packet
     * @param data data to add to the packet
     * @param reliable if reliable it will ensure the packet arrives
     * @return
     */
    template <typename TPacketId, typename TData>
    static ENetPacket* CreatePacket(TPacketId id, const TData& data, const bool reliable) {
        static_assert(std::is_integral_v<TPacketId>, "Packet ID must be an integral type");

        Buffer payload;
        if (glz::write_beve(data, payload)) {
            payload.clear();
        }

        return CreateRawPacket(static_cast<uint16_t>(id),
                               payload.empty() ? nullptr
                                               : reinterpret_cast<const uint8_t*>(payload.data()),
                               payload.size(), reliable);
    }

    /**
     * @brief Builds a packet from already-serialized payload bytes (Lua JSON tables,
     * raw strings). Prepends the 2-byte little-endian packet ID header.
     */
    static ENetPacket* CreateRawPacket(std::uint16_t id, const uint8_t* payload,
                                       std::size_t payloadSize, bool reliable) {
        Buffer buffer;
        buffer.reserve(sizeof(uint16_t) + payloadSize);
        buffer.push_back(static_cast<std::byte>(id & 0xFF));
        buffer.push_back(static_cast<std::byte>((id >> 8) & 0xFF));
        if (payload != nullptr && payloadSize > 0) {
            const auto* bytes = reinterpret_cast<const std::byte*>(payload);
            buffer.insert(buffer.end(), bytes, bytes + payloadSize);
        }
        const enet_uint32 flags = reliable ? ENET_PACKET_FLAG_RELIABLE : 0;
        return enet_packet_create(buffer.data(), buffer.size(), flags);
    }

    static ENetPacket* CreateRawPacket(std::uint16_t id, const Buffer& payload, bool reliable) {
        return CreateRawPacket(
            id, payload.empty() ? nullptr : reinterpret_cast<const uint8_t*>(payload.data()),
            payload.size(), reliable);
    }

    // disable host copy
    NetworkHostResource(NetworkHostResource&& other) noexcept
        : m_host(other.m_host), server_peer(other.server_peer), m_isServer(other.m_isServer),
          m_nextNetworkId(other.m_nextNetworkId), m_localNetworkId(other.m_localNetworkId),
          m_localEntity(other.m_localEntity), m_idToEntity(std::move(other.m_idToEntity)),
          m_entityToPeer(std::move(other.m_entityToPeer)),
          m_eventQueue(std::move(other.m_eventQueue)) {
        other.m_host = nullptr;
        other.server_peer = nullptr;
    }

    NetworkHostResource& operator=(NetworkHostResource&& other) noexcept {
        if (this != &other) {
            if (m_host) {
                enet_host_destroy(m_host);
            }
            m_host = other.m_host;
            server_peer = other.server_peer;
            m_isServer = other.m_isServer;
            m_nextNetworkId = other.m_nextNetworkId;
            m_localNetworkId = other.m_localNetworkId;
            m_localEntity = other.m_localEntity;
            m_idToEntity = std::move(other.m_idToEntity);
            m_entityToPeer = std::move(other.m_entityToPeer);
            m_eventQueue = std::move(other.m_eventQueue);
            other.m_host = nullptr;
            other.server_peer = nullptr;
        }
        return *this;
    }

    ~NetworkHostResource();

  private:
    NetworkHostResource(const ENetAddress* address, const size_t peerCount, const size_t channels,
                        bool isServer) {
        m_host = enet_host_create(address, peerCount, channels, 0, 0);
        if (!m_host) {
            throw std::runtime_error("Failed to create ENet host");
        }
        m_isServer = isServer;
    }

    /**
     * Only filled on the client to have access to the server to send data
     */
    ENetPeer* server_peer = nullptr;
    ENetHost* m_host = nullptr;
    bool m_isServer = false;
    std::size_t m_nextNetworkId = 1;
    std::optional<std::size_t> m_localNetworkId;
    std::optional<Entity> m_localEntity;
    std::unordered_map<std::size_t, Entity> m_idToEntity;
    std::unordered_map<Entity, ENetPeer*> m_entityToPeer;
    std::vector<ConnectionEvent> m_eventQueue;
};
} // namespace Engine::Multiplayer
#endif // G_ING_500_BAR_5_1_RTYPE_1_NETWORKHOST_HPP
