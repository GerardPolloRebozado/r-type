#ifndef OPENFRONT_NETWORK_CLIENT_HPP_
#define OPENFRONT_NETWORK_CLIENT_HPP_

#include "Network/UdpClient.hpp"
#include "NetworkProtocol.hpp"
#include "Simulation.hpp"
#include <functional>
#include <string>

namespace Openfront {

enum class ClientState { Disconnected, Connecting, Connected, InLobby, InGame };

class NetworkClient {
  public:
    NetworkClient();
    ~NetworkClient();

    bool Connect(const std::string& hostAddress, uint16_t port = kDefaultPort,
                 const std::string& playerName = "Player");
    void Disconnect();

    [[nodiscard]] bool IsConnected() const {
        return state_ != ClientState::Disconnected && state_ != ClientState::Connecting;
    }
    [[nodiscard]] bool HasJoined() const {
        return state_ == ClientState::InLobby || state_ == ClientState::InGame;
    }
    [[nodiscard]] bool IsInLobby() const {
        return state_ == ClientState::InLobby;
    }
    [[nodiscard]] bool IsInGame() const {
        return state_ == ClientState::InGame;
    }
    [[nodiscard]] const std::vector<LobbyPlayerInfo>& GetLobbyPlayers() const {
        return lobbyPlayers_;
    }
    [[nodiscard]] ClientState GetState() const {
        return state_;
    }
    [[nodiscard]] PlayerId GetLocalPlayerId() const {
        return localPlayerId_;
    }
    [[nodiscard]] uint32_t GetCurrentTurn() const {
        return currentTurn_;
    }
    [[nodiscard]] uint32_t GetPing() const;

    void SendIntent(const ClientIntent& intent);
    void PollNetwork(Simulation& sim, float dt);

    // Callbacks
    void SetOnJoinedCallback(std::function<void()> cb) {
        onJoined_ = std::move(cb);
    }
    void SetOnLobbyUpdatedCallback(std::function<void()> cb) {
        onLobbyUpdated_ = std::move(cb);
    }
    void SetOnGameStartedCallback(std::function<void()> cb) {
        onGameStarted_ = std::move(cb);
    }
    void SetOnDisconnectedCallback(std::function<void()> cb) {
        onDisconnected_ = std::move(cb);
    }

  private:
    Engine::Network::UdpClient udpClient_;
    ClientState state_{ClientState::Disconnected};

    std::string playerName_{"Player"};
    PlayerId localPlayerId_{kNeutralPlayerId};
    uint32_t currentTurn_{0};
    float pingTimer_{0.0f};

    std::vector<LobbyPlayerInfo> lobbyPlayers_;
    std::function<void()> onJoined_;
    std::function<void()> onLobbyUpdated_;
    std::function<void()> onGameStarted_;
    std::function<void()> onDisconnected_;

    void HandlePacket(Simulation& sim, const uint8_t* data, size_t size);
    void SendPacket(const ByteWriter& writer, bool reliable = true);
};

} // namespace Openfront

#endif // OPENFRONT_NETWORK_CLIENT_HPP_
