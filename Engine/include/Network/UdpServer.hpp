#ifndef ENGINE_NETWORK_UDP_SERVER_HPP_
#define ENGINE_NETWORK_UDP_SERVER_HPP_

#include "Compat/EnetWrapper.hpp"
#include "Network/ByteStream.hpp"
#include "Network/UdpPacket.hpp"
#include <atomic>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

namespace Engine::Network {

/**
 * @brief Multi-threaded high-performance UDP Game Server built on ENet.
 *
 * Runs network I/O asynchronously on a dedicated worker thread, buffering
 * network events in a thread-safe queue. The game simulation loop can poll
 * events without ever blocking frame execution or risking packet loss.
 */
class UdpServer {
  public:
    UdpServer();
    virtual ~UdpServer();

    // Non-copyable
    UdpServer(const UdpServer&) = delete;
    UdpServer& operator=(const UdpServer&) = delete;

    /**
     * @brief Starts the UDP server on the specified port.
     * @param port UDP port to listen on.
     * @param maxClients Maximum concurrent client connections.
     * @param channels Number of ENet channels.
     * @param multiThreaded When true, spawns a background I/O thread for network polling.
     * @return True if bind and startup succeeded.
     */
    bool Start(uint16_t port, size_t maxClients = 32, size_t channels = 2,
               bool multiThreaded = true);

    /**
     * @brief Stops the server, disconnects all peers, and joins the I/O thread.
     */
    void Stop();

    [[nodiscard]] bool IsRunning() const noexcept {
        return isRunning_.load();
    }
    [[nodiscard]] uint16_t GetPort() const noexcept {
        return port_;
    }
    [[nodiscard]] size_t GetConnectedCount() const;

    /**
     * @brief Drains all queued incoming network events from the I/O thread to the main thread.
     * @param handler Callback invoked for each received event.
     */
    void PollEvents(const std::function<void(const NetworkEvent&)>& handler);

    /**
     * @brief Sends a packet to a specific connected peer. Thread-safe.
     */
    void SendPacket(ENetPeer* peer, const uint8_t* data, size_t size, bool reliable = true,
                    uint8_t channel = 0);
    void SendPacket(ENetPeer* peer, const ByteWriter& writer, bool reliable = true,
                    uint8_t channel = 0);

    /**
     * @brief Broadcasts a packet to all connected peers. Thread-safe.
     */
    void BroadcastPacket(const uint8_t* data, size_t size, bool reliable = true,
                         uint8_t channel = 0);
    void BroadcastPacket(const ByteWriter& writer, bool reliable = true, uint8_t channel = 0);

    /**
     * @brief Disconnects a specific peer cleanly.
     */
    void DisconnectPeer(ENetPeer* peer, uint32_t data = 0);

  protected:
    void NetworkIoLoop();
    void ProcessEnetEvents();

    ENetHost* serverHost_{nullptr};
    uint16_t port_{0};
    bool multiThreaded_{true};

    std::atomic<bool> isRunning_{false};
    std::thread ioThread_;

    mutable std::mutex queueMutex_;
    std::queue<NetworkEvent> incomingEvents_;

    mutable std::mutex sendMutex_;
};

} // namespace Engine::Network

#endif // ENGINE_NETWORK_UDP_SERVER_HPP_
