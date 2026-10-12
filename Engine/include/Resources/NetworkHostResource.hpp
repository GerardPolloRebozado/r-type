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
     * @param timeoutMs time to wait till an event happens 0 to return inmediately
     * @param router router to use when a new packet is received
     * @param registry of the game
     */
    void Poll(uint32_t timeoutMs, Packets::PacketRouterResource router, Registry& registry);

    /**
     *  @brief This function will send any data to the specified peer, if the packet ID does not
     * match the struct being send the router will not be able to process it. Reliable should only
     * be used outside of the game, for example if we are just sending the player inputs we dont
     * need to set it to reliable
     * @tparam TPacketId type of the ID of the type of packet being send DON'T set it manually use
     * the function GetPacketTypeID<T>()
     * @tparam TData type of data that we want to send
     * @param peer where do we want to send it
     * @param id ID of the type of packet being send DON'T set it manually use the function
     * GetPacketTypeID<T>()
     * @param data data to send
     * @param channel the channel where the data would be send
     * @param reliable if we want to ensure the packet is delivered correctly or not
     */
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

    /**
     *  @brief This function will send any data to the specified peer, if the packet ID does not
     * match the struct being send the router will not be able to process it. Reliable should only
     * be used outside of the game, for example if we are just sending the player inputs we dont
     * need to set it to reliable
     * @tparam TPacketId type of the ID of the type of packet being send DON'T set it manually use
     * the function GetPacketTypeID<T>()
     * @tparam TData type of data that we want to send
     * @param id ID of the type of packet being send DON'T set it manually use the function
     * GetPacketTypeID<T>()
     * @param data data to send
     * @param channel the channel where the data would be send
     * @param reliable if we want to ensure the packet is delivered correctly or not
     */
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