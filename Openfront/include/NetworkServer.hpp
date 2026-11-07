#ifndef OPENFRONT_NETWORK_SERVER_HPP_
#define OPENFRONT_NETWORK_SERVER_HPP_

#include "Network/UdpServer.hpp"
#include "NetworkProtocol.hpp"
#include "Simulation.hpp"
#include <string>
#include <unordered_map>
#include <vector>

namespace Openfront {

struct ServerClientInfo {
    ENetPeer* peer{nullptr};
    PlayerId playerId{kNeutralPlayerId};
    std::string name;
    bool joined{false};
};

class NetworkServer {
  public:
    NetworkServer();
    ~NetworkServer();

    bool Start(uint16_t port = kDefaultPort, int maxClients = 16);
    void Stop();
    [[nodiscard]] bool IsRunning() const {
        return udpServer_.IsRunning();
    }
    [[nodiscard]] bool IsGameStarted() const {
        return isGameStarted_;
    }

    void SetHostName(const std::string& name) {
        hostName_ = name;
    }
    [[nodiscard]] const std::string& GetHostName() const {
        return hostName_;
    }

    void StartMatch(Simulation& sim);
    void BroadcastLobbyState();
    [[nodiscard]] std::vector<LobbyPlayerInfo> GetLobbyPlayers() const;
    void SendFullSync(ENetPeer* peer, const Simulation& sim);

    void QueueLocalIntent(const ClientIntent& intent);
    void PollNetwork(Simulation& sim, float dt);
    void TickTurn(Simulation& sim);

    [[nodiscard]] uint16_t GetPort() const {
        return udpServer_.GetPort();
    }
    [[nodiscard]] size_t GetConnectedClientCount() const {
        return clients_.size();
    }
    [[nodiscard]] const std::unordered_map<ENetPeer*, ServerClientInfo>& GetClients() const {
        return clients_;
    }

    void SetMatchSettings(uint32_t seed, uint8_t nations, uint8_t bots, float duration) {
        seed_ = seed;
        nations_ = nations;
        bots_ = bots;
        duration_ = duration;
    }

  private:
    Engine::Network::UdpServer udpServer_; // Powered by the Engine multi-threaded UDP server
    std::unordered_map<ENetPeer*, ServerClientInfo> clients_;
    std::vector<ClientIntent> pendingIntents_;

    uint32_t seed_{1337};
    uint8_t nations_{4};
    uint8_t bots_{4};
    float duration_{1800.0f};
    uint32_t turnNumber_{0};
    PlayerId nextPlayerId_{2}; // Player 1 is Host/Server
    bool isGameStarted_{false};
    std::string hostName_{"Host"};

    void HandlePacket(Simulation& sim, ENetPeer* peer, const uint8_t* data, size_t size);
    void SendPacket(ENetPeer* peer, const ByteWriter& writer, bool reliable = true);
    void BroadcastPacket(const ByteWriter& writer, bool reliable = true);
};

} // namespace Openfront

#endif // OPENFRONT_NETWORK_SERVER_HPP_
