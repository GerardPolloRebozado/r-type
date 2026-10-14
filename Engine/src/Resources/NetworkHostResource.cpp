/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** NetworkHost.cpp
*/

#include "Resources/NetworkHostResource.hpp"
#include "Resources/PacketRouterResource.hpp"

#include "ECS/Registry.hpp"
#include "enet.h"

#include <iostream>
#include <span>

namespace Engine::Multiplayer {
NetworkHostResource NetworkHostResource::CreateServer(const std::uint16_t port,
                                                      const std::size_t maxClients,
                                                      const std::size_t channels) {
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = port;
    return {&address, maxClients, channels, true};
}

NetworkHostResource NetworkHostResource::CreateClient(size_t channels) {
    return {nullptr, 1, channels, false};
}

NetworkHostResource::~NetworkHostResource() {
    if (m_host) {
        enet_host_destroy(m_host);
    }
}

ENetPeer* NetworkHostResource::Connect(const std::string_view host, const uint16_t port,
                                       const size_t channels) {
    ENetAddress address;
    enet_address_set_host(&address, host.data());
    address.port = port;
    server_peer = enet_host_connect(m_host, &address, channels, 0);
    return server_peer;
};

void NetworkHostResource::RegisterMultiplayerComponents(Registry& registry) {
    if (!registry.IsComponentRegistered<PlayerInformation>()) {
        registry.RegisterComponent<PlayerInformation>();
    }
    if (!registry.IsComponentRegistered<YourPlayerId>()) {
        registry.RegisterComponent<YourPlayerId>();
    }
}

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
    RegisterMultiplayerComponents(registry);
    if (registry.HasComponent<PlayerInformation>(entity)) {
        registry.GetComponent<PlayerInformation>(entity).id = networkId;
    } else {
        registry.AddComponent<PlayerInformation>(entity, PlayerInformation{networkId});
    }
    m_idToEntity[networkId] = entity;
    if (m_localNetworkId.has_value() && *m_localNetworkId == networkId) {
        m_localEntity = entity;
    }
}

void NetworkHostResource::Poll(uint32_t timeoutMs, PacketRouterResource router,
                               Registry& registry) {
    ENetEvent event;
    while (enet_host_service(m_host, &event, timeoutMs) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT: {
                if (m_isServer) {
                    RegisterMultiplayerComponents(registry);
                    const auto playerEntity = registry.CreateEntity();
                    const std::size_t networkId = m_nextNetworkId++;
                    registry.AddComponent(playerEntity, PlayerInformation{networkId});
                    m_idToEntity[networkId] = playerEntity;
                    m_entityToPeer[playerEntity] = event.peer;
                    SetPeerEntity(event.peer, playerEntity);
                    auto* packet = CreatePacket(YourPlayerIdPacket, YourPlayerId{networkId}, true);
                    Send(event.peer, packet, 0);
                }
                router.m_OnConnect(registry, event.peer);
                break;
            }
            case ENET_EVENT_TYPE_RECEIVE: {
                if (event.packet && event.packet->dataLength >= sizeof(PacketTypeID)) {
                    // The SDK owns the YourPlayerId handshake: remember our id and
                    // bind the entity as soon as the game spawns it, then still let
                    // game handlers run so existing code keeps working.
                    const auto packetId = static_cast<PacketTypeID>(event.packet->data[0] |
                                                                    (event.packet->data[1] << 8));
                    if (!m_isServer && packetId == YourPlayerIdPacket) {
                        YourPlayerId assigned{};
                        const std::span<const std::byte> payload{
                            reinterpret_cast<const std::byte*>(event.packet->data +
                                                               sizeof(PacketTypeID)),
                            event.packet->dataLength - sizeof(PacketTypeID)};
                        if (!glz::read_beve(assigned, payload)) {
                            m_localNetworkId = assigned.id;
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
                }
                enet_packet_destroy(event.packet);
                break;
            }
            case ENET_EVENT_TYPE_DISCONNECT: {
                if (const auto entity = GetEntityID(event.peer)) {
                    m_entityToPeer.erase(*entity);
                    if (registry.IsComponentRegistered<PlayerInformation>() &&
                        registry.HasComponent<PlayerInformation>(*entity)) {
                        const auto networkId = registry.GetComponent<PlayerInformation>(*entity).id;
                        m_idToEntity.erase(networkId);
                        if (m_localEntity.has_value() && *m_localEntity == *entity) {
                            m_localEntity.reset();
                        }
                        registry.DestroyEntity(*entity);
                    }
                    event.peer->data = nullptr;
                }
                std::cout << "New disconnection" << std::endl;
                router.m_OnDisconnect(registry, event.peer);
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
