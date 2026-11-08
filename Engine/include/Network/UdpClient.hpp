#ifndef ENGINE_NETWORK_UDP_CLIENT_HPP_
#define ENGINE_NETWORK_UDP_CLIENT_HPP_

#include "Compat/EnetWrapper.hpp"
#include "Network/ByteStream.hpp"
#include "Network/UdpPacket.hpp"
#include <functional>
#include <string>

namespace Engine::Network {

enum class ConnectionState { Disconnected, Connecting, Connected };

/**
 * @brief High-performance UDP Client built on ENet.
 */
class UdpClient {
  public:
    UdpClient();
    virtual ~UdpClient();

    // Non-copyable
    UdpClient(const UdpClient&) = delete;
    UdpClient& operator=(const UdpClient&) = delete;

    /**
     * @brief Initiates asynchronous connection to a remote UDP server.
     */
    bool Connect(const std::string& hostAddress, uint16_t port, size_t channels = 2);

    /**
     * @brief Cleanly disconnects from the server and releases host resources.
     */
    void Disconnect();

    [[nodiscard]] bool IsConnected() const noexcept {
        return state_ == ConnectionState::Connected;
    }
    [[nodiscard]] ConnectionState GetState() const noexcept {
        return state_;
    }
    [[nodiscard]] uint32_t GetPing() const noexcept;

    /**
     * @brief Polls incoming network packets from the server.
     * @param handler Callback invoked for connection, receive, and disconnect events.
     */
    void PollEvents(const std::function<void(const NetworkEvent&)>& handler);

    /**
     * @brief Sends a packet to the connected server peer.
     */
    void SendPacket(const uint8_t* data, size_t size, bool reliable = true, uint8_t channel = 0);
    void SendPacket(const ByteWriter& writer, bool reliable = true, uint8_t channel = 0);

  private:
    ENetHost* clientHost_{nullptr};
    ENetPeer* serverPeer_{nullptr};
    ConnectionState state_{ConnectionState::Disconnected};
};

} // namespace Engine::Network

#endif // ENGINE_NETWORK_UDP_CLIENT_HPP_
