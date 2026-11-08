#include "NetworkManager.hpp"
#include "Logging/Logger.hpp"
#include <iostream>

namespace Openfront {

NetworkManager::NetworkManager() = default;

NetworkManager::~NetworkManager() {
    Stop();
}

bool NetworkManager::StartSinglePlayer(Simulation& sim, MapType mapType, int spawnX, int spawnY) {
    Stop();
    mode_ = GameMode::SinglePlayer;
    sim.Init(4, 4, 1337, mapType, spawnX, spawnY);
    turnAccumulator_ = 0.0f;
    return true;
}

bool NetworkManager::StartHost(Simulation& sim, uint16_t port, const std::string& hostName,
                               MapType mapType, int spawnX, int spawnY) {
    Stop();
    uint32_t seed = 4242;
    sim.Init(4, 4, seed, mapType, spawnX, spawnY);
    sim.AssignPlayerSlot(1, hostName, FactionType::Human);

    server_.SetHostName(hostName);
    server_.SetMatchSettings(seed, 4, 4, sim.GetMatchDuration());
    if (!server_.Start(port, 16)) {
        return false;
    }

    mode_ = GameMode::Host;
    turnAccumulator_ = 0.0f;
    log_info("[NetworkManager] Hosted lobby on port " + std::to_string(port) + " as '" + hostName +
             "'");
    return true;
}

void NetworkManager::StartHostMatch(Simulation& sim) {
    if (mode_ == GameMode::Host) {
        server_.StartMatch(sim);
        turnAccumulator_ = 0.0f;
    }
}

bool NetworkManager::StartClient(Simulation& sim, const std::string& address, uint16_t port,
                                 const std::string& clientName) {
    Stop();
    mode_ = GameMode::Client;
    turnAccumulator_ = 0.0f;
    return client_.Connect(address, port, clientName);
}

void NetworkManager::Stop() {
    if (mode_ == GameMode::Host) {
        server_.Stop();
    } else if (mode_ == GameMode::Client) {
        client_.Disconnect();
    }
    mode_ = GameMode::SinglePlayer;
    turnAccumulator_ = 0.0f;
}

void NetworkManager::Update(Simulation& sim, float dt) {
    switch (mode_) {
        case GameMode::SinglePlayer: {
            turnAccumulator_ += dt * gameSpeed_;
            while (turnAccumulator_ >= kSimulationTickDt) {
                std::vector<ClientIntent> intents;
                sim.CollectAIIntents(intents, kSimulationTickDt);
                sim.ExecuteTurn(intents, kSimulationTickDt);
                turnAccumulator_ -= kSimulationTickDt;
            }
            break;
        }
        case GameMode::Host: {
            server_.PollNetwork(sim, dt);
            if (server_.IsGameStarted()) {
                turnAccumulator_ += dt * gameSpeed_;
                while (turnAccumulator_ >= kSimulationTickDt) {
                    server_.TickTurn(sim);
                    turnAccumulator_ -= kSimulationTickDt;
                }
            }
            break;
        }
        case GameMode::Client: {
            client_.PollNetwork(sim, dt);
            break;
        }
        default:
            break;
    }
}

PlayerId NetworkManager::GetLocalPlayerId() const {
    if (mode_ == GameMode::Client) {
        return client_.GetLocalPlayerId();
    }
    return 1;
}

uint32_t NetworkManager::GetPing() const {
    if (mode_ == GameMode::Client) {
        return client_.GetPing();
    }
    return 0;
}

uint32_t NetworkManager::GetCurrentTurn() const {
    if (mode_ == GameMode::Client) {
        return client_.GetCurrentTurn();
    }
    return 0;
}

size_t NetworkManager::GetConnectedPlayersCount() const {
    if (mode_ == GameMode::Host) {
        return server_.GetLobbyPlayers().size();
    }
    if (mode_ == GameMode::Client) {
        return client_.GetLobbyPlayers().empty() ? (client_.IsConnected() ? 2 : 1)
                                                 : client_.GetLobbyPlayers().size();
    }
    return 1;
}

void NetworkManager::QueueAttack(Simulation& sim, PlayerId id, int startX, int startY, int targetX,
                                 int targetY, float ratio) {
    if (mode_ == GameMode::SinglePlayer) {
        sim.LaunchAttack(id, startX, startY, targetX, targetY, ratio);
        return;
    }

    ClientIntent it;
    it.playerId = id;
    it.type = IntentType::Attack;
    it.startX = startX;
    it.startY = startY;
    it.targetX = targetX;
    it.targetY = targetY;
    it.ratio = ratio;

    if (mode_ == GameMode::Host) {
        server_.QueueLocalIntent(it);
    } else if (mode_ == GameMode::Client) {
        client_.SendIntent(it);
    }
}

void NetworkManager::QueueReinforce(Simulation& sim, PlayerId id, uint32_t frontId) {
    if (mode_ == GameMode::SinglePlayer) {
        sim.ReinforceFront(id, frontId);
        return;
    }

    ClientIntent it;
    it.playerId = id;
    it.type = IntentType::Reinforce;
    it.frontId = frontId;

    if (mode_ == GameMode::Host) {
        server_.QueueLocalIntent(it);
    } else if (mode_ == GameMode::Client) {
        client_.SendIntent(it);
    }
}

void NetworkManager::QueueBuild(Simulation& sim, PlayerId id, int x, int y, BuildingType type) {
    if (mode_ == GameMode::SinglePlayer) {
        sim.Build(id, x, y, type);
        return;
    }

    ClientIntent it;
    it.playerId = id;
    it.type = IntentType::Build;
    it.startX = x;
    it.startY = y;
    it.buildingType = type;

    if (mode_ == GameMode::Host) {
        server_.QueueLocalIntent(it);
    } else if (mode_ == GameMode::Client) {
        client_.SendIntent(it);
    }
}

void NetworkManager::QueueLaunchNuke(Simulation& sim, PlayerId id, int siloX, int siloY,
                                     int targetX, int targetY, BombType type) {
    if (mode_ == GameMode::SinglePlayer) {
        sim.LaunchNuke(id, siloX, siloY, targetX, targetY, type);
        return;
    }

    ClientIntent it;
    it.playerId = id;
    it.type = IntentType::LaunchNuke;
    it.startX = siloX;
    it.startY = siloY;
    it.targetX = targetX;
    it.targetY = targetY;
    it.bombType = type;

    if (mode_ == GameMode::Host) {
        server_.QueueLocalIntent(it);
    } else if (mode_ == GameMode::Client) {
        client_.SendIntent(it);
    }
}

void NetworkManager::QueueSetAttackRatio(Simulation& sim, PlayerId id, float ratio) {
    if (mode_ == GameMode::SinglePlayer) {
        Player* p = sim.GetPlayer(id);
        if (p) {
            p->attackRatio = ratio;
        }
        return;
    }

    ClientIntent it;
    it.playerId = id;
    it.type = IntentType::SetAttackRatio;
    it.ratio = ratio;

    if (mode_ == GameMode::Host) {
        server_.QueueLocalIntent(it);
    } else if (mode_ == GameMode::Client) {
        client_.SendIntent(it);
    }
}

} // namespace Openfront
