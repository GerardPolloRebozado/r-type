/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** NetworkHost.cpp
*/

#include "Resources/NetworkHostResource.hpp"
#include "Resources/PacketRouterResource.hpp"

#include "Compat/EnetWrapper.hpp"
#include "ECS/Registry.hpp"
#include "Logging/Logger.hpp"

#include <mutex>
#include <span>

namespace Engine::Multiplayer {

namespace {

/**
 * ENet must be initialized exactly once before any host is created. The
 * editor-generated game entry point never calls enet_initialize() itself, so
 * the SDK does it lazily here, no matter who creates the first host.
 */
void EnsureENetInitialized() {
    static std::once_flag initialized;
    std::call_once(initialized, [] {
        if (enet_initialize() != 0) {
            throw std::runtime_error("Failed to initialize ENet.");
        }
    });
}

} // namespace

NetworkHostResource NetworkHostResource::CreateServer(const std::uint16_t port,
                                                      const std::size_t maxClients,
                                                      const std::size_t channels) {
    EnsureENetInitialized();
    ENetAddress address{};
    address.host = ENET_HOST_ANY;
    address.port = port;
    return {&address, maxClients, channels, true};
}

NetworkHostResource NetworkHostResource::CreateClient(size_t channels) {
    EnsureENetInitialized();
    return {nullptr, 1, channels, false};
}

NetworkHostResource::~NetworkHostResource() {
    if (m_host) {
        enet_host_destroy(m_host);
    }
}

ENetPeer* NetworkHostResource::Connect(const std::string_view host, const uint16_t port,
                                       const size_t channels) {
    ENetAddress address{};
    const std::string hostStr(
        host); // needed to avoid windows crash because string_view may not have null terminal
    enet_address_set_host(&address, hostStr.c_str());
    address.port = port;
    server_peer = enet_host_connect(m_host, &address, channels, 0);
    return server_peer;
};

std::optional<std::size_t> NetworkHostResource::GetNetworkId(Registry& registry,
                                                             Entity entity) const {
    if (!registry.IsComponentRegistered<PlayerInformation>()) {
        return std::nullopt;
    }
    if (!registry.HasComponent<PlayerInformation>(entity)) {
        return std::nullopt;
    }
    return registry.GetComponent<PlayerInformation>(entity).id;
}

std::optional<Entity> NetworkHostResource::GetEntityByNetworkId(std::size_t networkId) const {
    const auto it = m_idToEntity.find(networkId);
    if (it == m_idToEntity.end()) {
        return std::nullopt;
    }
    return it->second;
}

std::optional<ENetPeer*> NetworkHostResource::GetPeer(Entity entity) const {
    const auto it = m_entityToPeer.find(entity);
    if (it == m_entityToPeer.end()) {
        return std::nullopt;
    }
    return it->second;
}

void NetworkHostResource::TrackNetworkEntity(Registry& registry, Entity entity,
                                             std::size_t networkId) {
    if (registry.HasComponent<PlayerInformation>(entity)) {
        registry.GetComponent<PlayerInformation>(entity).id = networkId;
    } else {
        registry.AddComponent<PlayerInformation>(entity, PlayerInformation{networkId});
    }
    m_idToEntity[networkId] = entity;
    if (m_localNetworkId.has_value() && *m_localNetworkId == networkId) {
        m_localEntity = entity;
        log_info("[Network] Bound local entity " + std::to_string(entity) + " to network id " +
                 std::to_string(networkId));
    }
}

void NetworkHostResource::UntrackNetworkEntity(Registry& registry, Entity entity) {
    if (registry.IsComponentRegistered<PlayerInformation>() &&
        registry.HasComponent<PlayerInformation>(entity)) {
        m_idToEntity.erase(registry.GetComponent<PlayerInformation>(entity).id);
    }
    m_entityToPeer.erase(entity);
    if (m_localEntity.has_value() && *m_localEntity == entity) {
        m_localEntity.reset();
    }
}

void NetworkHostResource::Poll(uint32_t timeoutMs, const PacketRouterResource& router,
                               Registry& registry) {
    ENetEvent event;
    auto& packetRegistry = registry.GetResource<PacketRegistryResource>();
    const auto YourPlayerIdPacketID = packetRegistry.GetOrRegister("your_player_id");
    while (enet_host_service(m_host, &event, timeoutMs) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT: {
                // Only the server spawns a player per connection
                if (m_isServer) {
                    const auto playerEntity = registry.CreateEntity();
                    const std::size_t networkId = m_nextNetworkId++;
                    registry.AddComponent(playerEntity, PlayerInformation{networkId});
                    m_idToEntity[networkId] = playerEntity;
                    m_entityToPeer[playerEntity] = event.peer;
                    SetPeerEntity(event.peer, playerEntity);
                    auto* packet =
                        CreatePacket(YourPlayerIdPacketID, YourPlayerId{networkId}, true);
                    Send(event.peer, packet, 0);
                    QueueEvent(ConnectionEvent::Type::Connect, event.peer, playerEntity, networkId);
                }
                if (router.m_OnConnect) {
                    router.m_OnConnect(registry, event.peer);
                }
                break;
            }
            case ENET_EVENT_TYPE_RECEIVE: {
                if (event.packet && event.packet->dataLength >= sizeof(PacketTypeID)) {
                    // The SDK owns the YourPlayerId handshake: remember our id and
                    // bind the entity as soon as the game spawns it, then still let
                    // game handlers run so existing code keeps working.
                    const auto packetId = static_cast<PacketTypeID>(event.packet->data[0] |
                                                                    (event.packet->data[1] << 8));
                    if (!m_isServer && packetId == YourPlayerIdPacketID) {
                        YourPlayerId assigned{};
                        const std::span<const std::byte> payload{
                            reinterpret_cast<const std::byte*>(event.packet->data +
                                                               sizeof(PacketTypeID)),
                            event.packet->dataLength - sizeof(PacketTypeID)};
                        if (!glz::read_beve(assigned, payload)) {
                            m_localNetworkId = assigned.id;
                            log_info("[Network] Assigned local network id " +
                                     std::to_string(assigned.id));
                        }
                    }
                    router.Dispatch(registry, event.peer, event.packet->data,
                                    event.packet->dataLength);
                    if (!m_isServer && m_localNetworkId.has_value() && !m_localEntity.has_value() &&
                        registry.IsComponentRegistered<YourPlayerId>()) {
                        for (const auto& [entity, marker] :
                             *registry.GetComponentArray<YourPlayerId>()) {
                            if (marker.id == *m_localNetworkId) {
                                TrackNetworkEntity(registry, entity, *m_localNetworkId);
                                break;
                            }
                        }
                    }
                    // The server may have spawned our replica before we learned
                    // our id
                    if (!m_isServer && m_localNetworkId.has_value() && !m_localEntity.has_value()) {
                        if (auto it = m_idToEntity.find(*m_localNetworkId);
                            it != m_idToEntity.end()) {
                            m_localEntity = it->second;
                        }
                    }
                }
                enet_packet_destroy(event.packet);
                break;
            }
            case ENET_EVENT_TYPE_DISCONNECT: {
                const auto entity = GetEntityID(event.peer);
                std::size_t networkId = 0;
                if (entity.has_value() && registry.IsComponentRegistered<PlayerInformation>() &&
                    registry.HasComponent<PlayerInformation>(*entity)) {
                    networkId = registry.GetComponent<PlayerInformation>(*entity).id;
                }
                QueueEvent(ConnectionEvent::Type::Disconnect, event.peer,
                           entity.value_or(static_cast<Entity>(-1)), networkId);
                if (router.m_OnDisconnect) {
                    router.m_OnDisconnect(registry, event.peer);
                }
                if (entity.has_value()) {
                    m_entityToPeer.erase(*entity);
                    if (registry.IsComponentRegistered<PlayerInformation>() &&
                        registry.HasComponent<PlayerInformation>(*entity)) {
                        m_idToEntity.erase(networkId);
                        if (m_localEntity.has_value() && *m_localEntity == *entity) {
                            m_localEntity.reset();
                        }
                        registry.DestroyEntity(*entity);
                    }
                    event.peer->data = nullptr;
                }
                log_info("[Network] Peer disconnected" +
                         (networkId != 0 ? " (network id " + std::to_string(networkId) + ")"
                                         : std::string{}));
                break;
            }
            default:
                break;
        }
        timeoutMs = 0; // Poll remaining queued events without blocking
    }
}

std::optional<Entity> NetworkHostResource::GetEntityID(ENetPeer* peer) {
    if (peer && peer->data) {
        return reinterpret_cast<size_t>(peer->data);
    }
    return std::nullopt;
}

void NetworkHostResource::SetPeerEntity(ENetPeer* peer, const Entity entity) {
    if (peer) {
        enet_peer_set_data(peer, reinterpret_cast<void*>(entity));
    }
}
}; // namespace Engine::Multiplayer
