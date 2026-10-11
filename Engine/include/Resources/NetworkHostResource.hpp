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

#include <enet.h>
#include <stdexcept>
#include <string_view>

namespace Engine::Multiplayer {

class NetworkHostResource {
  public:
    static NetworkHostResource CreateServer(std::uint16_t port, std::size_t maxClients = 32,
                                            std::size_t channels = 2);

    static NetworkHostResource CreateClient(size_t channels = 2);

    ENetPeer* Connect(std::string_view host, uint16_t port, size_t channels = 2);

    void Poll(uint32_t timeoutMs, Packets::PacketRouterResource router, Registry& registry);

    template <typename TPacketId, typename TData>
    static void Send(ENetPeer* peer, TPacketId id, const TData& data, const uint8_t channel = 0,
                     const bool reliable = true) {
        static_assert(std::is_integral_v<TPacketId>, "Packet ID must be an integral type");

        Packets::Buffer buffer;
        auto writer = bitsery::Serializer<Packets::OutputAdapter>(buffer);

        auto rawId = static_cast<uint16_t>(id);
        writer.value2b(rawId);
        writer.object(data);
        writer.adapter().flush();

        const enet_uint32 flags = reliable ? ENET_PACKET_FLAG_RELIABLE : 0;
        ENetPacket* packet = enet_packet_create(buffer.data(), buffer.size(), flags);
        enet_peer_send(peer, channel, packet);
    }

    template <class TPacketId, class TData>
    void Send(TPacketId id, const TData& data, uint8_t channel, bool reliable) {
        Send<TPacketId, TData>(server_peer, id, data, channel, reliable);
    }

    // disable host copy
    NetworkHostResource(NetworkHostResource&& other) noexcept : m_host(other.m_host) {
        other.m_host = nullptr;
    }

    NetworkHostResource& operator=(NetworkHostResource&& other) noexcept {
        if (this != &other) {
            if (m_host) {
                enet_host_destroy(m_host);
            }
            m_host = other.m_host;
            other.m_host = nullptr;
        }
        return *this;
    }

    ~NetworkHostResource();

  private:
    NetworkHostResource(const ENetAddress* address, const size_t peerCount, const size_t channels) {
        m_host = enet_host_create(address, peerCount, channels, 0, 0);
        if (!m_host) {
            throw std::runtime_error("Failed to create ENet host");
        }
    }

    /**
     * Only filled on the client to have access to the server to send data
     */
    ENetPeer* server_peer = nullptr;
    ENetHost* m_host = nullptr;
};
} // namespace Engine::Multiplayer
#endif // G_ING_500_BAR_5_1_RTYPE_1_NETWORKHOST_HPP