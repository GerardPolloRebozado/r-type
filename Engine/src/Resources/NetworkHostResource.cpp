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

namespace Engine::Multiplayer {
NetworkHostResource NetworkHostResource::CreateServer(const std::uint16_t port,
                                                      const std::size_t maxClients,
                                                      const std::size_t channels) {
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = port;
    return {&address, maxClients, channels};
}

NetworkHostResource NetworkHostResource::CreateClient(size_t channels) {
    return {nullptr, 1, channels};
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

void NetworkHostResource::Poll(uint32_t timeoutMs, PacketRouterResource router,
                               Registry& registry) {
    ENetEvent event;
    while (enet_host_service(m_host, &event, timeoutMs) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT: {
                const auto playerEntity = registry.CreateEntity();
                SetPeerEntity(event.peer, playerEntity);
                const auto playerInformation = PlayerInformation{GetNewPlayerId(), event.peer};
                registry.AddComponent(playerEntity, playerInformation);
                auto* packet =
                    CreatePacket(YourPlayerIdPacket, YourPlayerId{playerInformation.id}, true);
                Send(event.peer, packet, 0);
                router.m_OnConnect(registry, event.peer);
                break;
            }
            case ENET_EVENT_TYPE_RECEIVE:
                if (event.packet && event.packet->dataLength >= sizeof(PacketTypeID)) {
                    router.Dispatch(registry, event.peer, event.packet->data,
                                    event.packet->dataLength);
                }
                enet_packet_destroy(event.packet);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                std::cout << "New disconnection" << std::endl;
                router.m_OnDisconnect(registry, event.peer);
                break;
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
