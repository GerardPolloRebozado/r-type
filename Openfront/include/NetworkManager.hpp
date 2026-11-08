#ifndef OPENFRONT_NETWORK_MANAGER_HPP_
#define OPENFRONT_NETWORK_MANAGER_HPP_

#include "Common.hpp"
#include "NetworkClient.hpp"
#include "NetworkProtocol.hpp"
#include "NetworkServer.hpp"
#include "Simulation.hpp"
#include <memory>
#include <string>

namespace Openfront {

enum class GameMode { MainMenu, SinglePlayer, Host, Client, DedicatedServer };

class NetworkManager {
  public:
    NetworkManager();
    ~NetworkManager();

    bool StartSinglePlayer(Simulation& sim, MapType mapType = MapType::Procedural);
    bool StartHost(Simulation& sim, uint16_t port = kDefaultPort,
                   const std::string& hostName = "Host", MapType mapType = MapType::Procedural);
    void StartHostMatch(Simulation& sim);
    bool StartClient(Simulation& sim, const std::string& address, uint16_t port = kDefaultPort,
                     const std::string& clientName = "Player");
    void Stop();

    void Update(Simulation& sim, float dt);
    void SetGameSpeed(float speed) {
        gameSpeed_ = speed;
    }
    [[nodiscard]] float GetGameSpeed() const {
        return gameSpeed_;
    }

    // Intent queuing
    void QueueAttack(Simulation& sim, PlayerId id, int startX, int startY, int targetX, int targetY,
                     float ratio = -1.0f);
    void QueueReinforce(Simulation& sim, PlayerId id, uint32_t frontId);
    void QueueBuild(Simulation& sim, PlayerId id, int x, int y, BuildingType type);
    void QueueLaunchNuke(Simulation& sim, PlayerId id, int siloX, int siloY, int targetX,
                         int targetY, BombType type);
    void QueueSetAttackRatio(Simulation& sim, PlayerId id, float ratio);

    [[nodiscard]] GameMode GetMode() const {
        return mode_;
    }
    [[nodiscard]] PlayerId GetLocalPlayerId() const;
    [[nodiscard]] bool IsMultiplayer() const {
        return mode_ == GameMode::Host || mode_ == GameMode::Client;
    }
    [[nodiscard]] uint32_t GetPing() const;
    [[nodiscard]] uint32_t GetCurrentTurn() const;
    [[nodiscard]] size_t GetConnectedPlayersCount() const;

    NetworkServer& GetServer() {
        return server_;
    }
    const NetworkServer& GetServer() const {
        return server_;
    }
    NetworkClient& GetClient() {
        return client_;
    }
    const NetworkClient& GetClient() const {
        return client_;
    }

  private:
    GameMode mode_{GameMode::SinglePlayer};
    NetworkServer server_;
    NetworkClient client_;
    float turnAccumulator_{0.0f};
    float gameSpeed_{1.0f};
};

} // namespace Openfront

#endif // OPENFRONT_NETWORK_MANAGER_HPP_
