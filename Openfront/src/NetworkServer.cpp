#include "NetworkServer.hpp"
#include "Logging/Logger.hpp"
#include <iostream>

namespace Openfront {

NetworkServer::NetworkServer() = default;

NetworkServer::~NetworkServer() {
    Stop();
}

bool NetworkServer::Start(uint16_t port, int maxClients, ServerRole role) {
    if (udpServer_.IsRunning()) {
        return true;
    }
    if (maxClients < 1) {
        maxClients = 1;
    }

    if (!udpServer_.Start(port, static_cast<size_t>(maxClients), 2, true)) {
        log_error("[NetworkServer] Failed to create multi-threaded server on port " +
                  std::to_string(port));
        return false;
    }

    role_ = role;
    maxPlayers_ = static_cast<uint16_t>(maxClients);
    turnNumber_ = 0;
    isGameStarted_ = false;
    pendingIntents_.clear();
    clients_.clear();
    lobbyTimer_ = 0.0f;
    gameOverTimer_ = 0.0f;

    log_info("[NetworkServer] Multi-threaded server started on port " + std::to_string(port) +
             (role_ == ServerRole::Dedicated ? " (dedicated room '" + roomName_ + "', " +
                                                   std::to_string(maxClients) + " slots)"
                                             : " (lobby host mode)"));
    return true;
}

void NetworkServer::Stop() {
    if (!udpServer_.IsRunning()) {
        return;
    }

    udpServer_.Stop();
    clients_.clear();
    pendingIntents_.clear();
    isGameStarted_ = false;

    log_info("[NetworkServer] Server stopped.");
}

void NetworkServer::ConfigureRoom(const std::string& roomName, uint16_t maxPlayers, uint8_t bots,
                                  MapType mapType, float autoStartDelay) {
    roomName_ = roomName;
    maxPlayers_ = (maxPlayers < 1) ? 1 : maxPlayers;
    bots_ = bots;
    mapType_ = mapType;
    autoStartDelay_ = (autoStartDelay < 0.0f) ? 0.0f : autoStartDelay;
    nations_ = 4;
    if (maxPlayers_ > 12) {
        maxPlayers_ = 12;
    }
}

void NetworkServer::QueueLocalIntent(const ClientIntent& intent) {
    pendingIntents_.push_back(intent);
}

PlayerId NetworkServer::AllocatePlayerId() {
    for (uint16_t id = 2; id <= maxPlayers_ + 1; ++id) {
        bool taken = false;
        for (const auto& [peer, info] : clients_) {
            if (info.joined && info.playerId == id) {
                taken = true;
                break;
            }
        }
        if (!taken) {
            return id;
        }
    }
    return kNeutralPlayerId;
}

void NetworkServer::PollNetwork(Simulation& sim, float dt) {
    if (!udpServer_.IsRunning()) {
        return;
    }

    udpServer_.PollEvents([&](const Engine::Network::NetworkEvent& event) {
        switch (event.type) {
            case Engine::Network::NetworkEventType::Connect: {
                ServerClientInfo info;
                info.peer = event.peer;
                info.playerId = kNeutralPlayerId;
                info.joined = false;
                clients_[event.peer] = info;

                ByteWriter room;
                room.Write(static_cast<uint8_t>(PacketType::RoomInfo));
                room.WriteRoomInfo(GetRoomInfo());
                SendPacket(event.peer, room);

                char hostIp[64] = {0};
                enet_address_get_host_ip(&event.peer->address, hostIp, sizeof(hostIp));
                log_info("[NetworkServer] Client connected from " + std::string(hostIp));
                break;
            }
            case Engine::Network::NetworkEventType::Receive: {
                HandlePacket(sim, event.peer, event.data.data(), event.data.size());
                break;
            }
            case Engine::Network::NetworkEventType::Disconnect: {
                auto it = clients_.find(event.peer);
                if (it != clients_.end()) {
                    PlayerId pid = it->second.playerId;
                    bool wasJoined = it->second.joined;
                    log_info("[NetworkServer] Client " + std::to_string(pid) + " (" +
                             it->second.name + ") disconnected.");

                    clients_.erase(it);

                    if (wasJoined) {
                        if (isGameStarted_ && pid != kNeutralPlayerId) {
                            Player* p = sim.GetPlayer(pid);
                            if (p) {
                                p->type = FactionType::Bot;
                            }
                            ByteWriter writer;
                            writer.Write(static_cast<uint8_t>(PacketType::PlayerLeft));
                            writer.Write(pid);
                            BroadcastPacket(writer);
                        } else {
                            BroadcastLobbyState();
                        }
                    }
                }
                break;
            }
            default:
                break;
        }
    });

    UpdateRoom(sim, dt);
}

void NetworkServer::UpdateRoom(Simulation& sim, float dt) {
    if (!udpServer_.IsRunning() || role_ != ServerRole::Dedicated) {
        return;
    }

    lobbyTimer_ += dt;
    size_t joined = GetJoinedPlayersCount();

    if (!isGameStarted_) {
        bool roomFull = (joined >= maxPlayers_);
        bool waited = (joined >= 1 && lobbyTimer_ >= autoStartDelay_);
        if (roomFull || waited) {
            StartMatch(sim);
            lobbyTimer_ = 0.0f;
        }
        return;
    }

    if (sim.IsGameOver()) {
        gameOverTimer_ += dt;
        if (gameOverTimer_ >= 15.0f) {
            ResetRoomToLobby();
            BroadcastLobbyState();
            log_info("[NetworkServer] Room '" + roomName_ + "' reset to lobby.");
        }
        return;
    }
    gameOverTimer_ = 0.0f;
}

RoomInfo NetworkServer::GetRoomInfo() const {
    RoomInfo info;
    info.name = (role_ == ServerRole::Dedicated) ? roomName_
                                                 : (hostName_.empty() ? "Hosted Game" : hostName_);
    info.mapType = mapType_;
    info.players = static_cast<uint16_t>(GetJoinedPlayersCount());
    info.maxPlayers = maxPlayers_;
    info.gameStarted = isGameStarted_;
    info.turn = turnNumber_;
    return info;
}

uint16_t NetworkServer::GetJoinedPlayersCount() const {
    uint16_t count = 0;
    for (const auto& [peer, info] : clients_) {
        if (info.joined) {
            count++;
        }
    }
    return count;
}

void NetworkServer::HandlePacket(Simulation& sim, ENetPeer* peer, const uint8_t* data,
                                 size_t size) {
    if (!data || size == 0) {
        return;
    }

    ByteReader reader(data, size);
    uint8_t pktType = 0;
    if (!reader.Read(pktType)) {
        return;
    }

    auto type = static_cast<PacketType>(pktType);

    switch (type) {
        case PacketType::JoinRequest: {
            std::string playerName;
            if (!reader.ReadString(playerName)) {
                return;
            }

            auto it = clients_.find(peer);
            if (it == clients_.end()) {
                return;
            }

            PlayerId assignedId = AllocatePlayerId();
            if (assignedId == kNeutralPlayerId) {
                ByteWriter resp;
                resp.Write(static_cast<uint8_t>(PacketType::JoinResponse));
                resp.Write(false);
                SendPacket(peer, resp);
                return;
            }
            it->second.playerId = assignedId;
            it->second.name = playerName;
            it->second.joined = true;

            if (!isGameStarted_) {
                ByteWriter resp;
                resp.Write(static_cast<uint8_t>(PacketType::JoinResponse));
                resp.Write(true); // accepted
                resp.Write(assignedId);
                resp.Write(seed_);
                resp.Write(nations_);
                resp.Write(bots_);
                resp.Write(duration_);
                resp.Write(static_cast<uint32_t>(0));
                resp.Write(static_cast<uint8_t>(mapType_));
                SendPacket(peer, resp);

                BroadcastLobbyState();

                log_info("[NetworkServer] Client '" + playerName + "' joined " + roomName_ +
                         " as Player " + std::to_string(assignedId));
            } else {
                // Late join mid-game
                sim.CancelAllFrontsForPlayer(assignedId);
                sim.AssignPlayerSlot(assignedId, playerName, FactionType::Human);

                ByteWriter resp;
                resp.Write(static_cast<uint8_t>(PacketType::JoinResponse));
                resp.Write(true);
                resp.Write(assignedId);
                resp.Write(seed_);
                resp.Write(nations_);
                resp.Write(bots_);
                resp.Write(duration_);
                resp.Write(turnNumber_);
                resp.Write(static_cast<uint8_t>(mapType_));
                SendPacket(peer, resp);

                SendFullSync(peer, sim);

                const Player* p = sim.GetPlayer(assignedId);
                ByteWriter joined;
                joined.Write(static_cast<uint8_t>(PacketType::PlayerJoined));
                joined.Write(assignedId);
                joined.WriteString(playerName);
                joined.Write(p ? p->color : WHITE);
                BroadcastPacket(joined);

                log_info("[NetworkServer] Mid-match join: Player " + std::to_string(assignedId) +
                         " ('" + playerName + "') took over a slot in " + roomName_);
            }
            break;
        }
        case PacketType::ClientIntent: {
            ClientIntent intent;
            if (!reader.ReadIntent(intent)) {
                return;
            }

            // ensure intent playerId matches the sender peer's assigned ID
            auto it = clients_.find(peer);
            if (it != clients_.end() && it->second.playerId == intent.playerId) {
                pendingIntents_.push_back(intent);
            }
            break;
        }
        case PacketType::Ping: {
            ByteWriter pong;
            pong.Write(static_cast<uint8_t>(PacketType::Pong));
            SendPacket(peer, pong, false);
            break;
        }
        case PacketType::RoomInfo: {
            ByteWriter room;
            room.Write(static_cast<uint8_t>(PacketType::RoomInfo));
            room.WriteRoomInfo(GetRoomInfo());
            SendPacket(peer, room);
            break;
        }
        default:
            break;
    }
}

void NetworkServer::TickTurn(Simulation& sim) {
    if (!udpServer_.IsRunning() || !isGameStarted_ || sim.IsGameOver()) {
        return;
    }

    sim.CollectAIIntents(pendingIntents_, kSimulationTickDt);

    ByteWriter writer;
    writer.Write(static_cast<uint8_t>(PacketType::ServerTurn));
    writer.Write(turnNumber_);
    auto count = static_cast<uint16_t>(pendingIntents_.size());
    writer.Write(count);

    for (const auto& intent : pendingIntents_) {
        writer.WriteIntent(intent);
    }

    BroadcastPacket(writer, true);

    sim.ExecuteTurn(pendingIntents_, kSimulationTickDt);
    pendingIntents_.clear();

    turnNumber_++;
}

void NetworkServer::StartMatch(Simulation& sim) {
    if (!udpServer_.IsRunning() || isGameStarted_) {
        return;
    }

    isGameStarted_ = true;
    turnNumber_ = 0;
    pendingIntents_.clear();

    auto lobby = GetLobbyPlayers();
    if (role_ == ServerRole::Dedicated) {
        uint8_t humans = static_cast<uint8_t>(lobby.empty() ? 0 : lobby.size());
        sim.Init(static_cast<uint8_t>(humans + 1), bots_, seed_, mapType_);
        if (Player* leftover = sim.GetPlayer(1)) {
            leftover->name = "Bot Zero";
            leftover->type = FactionType::Nation;
        }
        for (const auto& p : lobby) {
            sim.CancelAllFrontsForPlayer(p.id);
            sim.AssignPlayerSlot(p.id, p.name, FactionType::Human);
        }
    } else {
        sim.Init(nations_, bots_, seed_, mapType_);
        sim.AssignPlayerSlot(1, hostName_.empty() ? "Commander" : hostName_, FactionType::Human);
        for (const auto& p : lobby) {
            if (!p.isHost) {
                sim.CancelAllFrontsForPlayer(p.id);
                sim.AssignPlayerSlot(p.id, p.name, FactionType::Human);
            }
        }
    }

    ByteWriter startMsg;
    startMsg.Write(static_cast<uint8_t>(PacketType::StartGame));
    startMsg.Write(seed_);
    startMsg.Write(nations_);
    startMsg.Write(bots_);
    startMsg.Write(duration_);
    startMsg.Write(static_cast<uint8_t>(mapType_));
    startMsg.Write(static_cast<uint16_t>(lobby.size()));
    for (const auto& p : lobby) {
        startMsg.WriteLobbyPlayer(p);
    }

    BroadcastPacket(startMsg, true);
    log_info("[NetworkServer] Match started in '" + roomName_ + "' with " +
             std::to_string(lobby.size()) + " human players on " + MapTypeName(mapType_));
}

void NetworkServer::BroadcastLobbyState() {
    if (!udpServer_.IsRunning()) {
        return;
    }

    ByteWriter writer;
    writer.Write(static_cast<uint8_t>(PacketType::LobbyState));
    writer.Write(seed_);
    writer.Write(nations_);
    writer.Write(bots_);

    auto lobby = GetLobbyPlayers();
    writer.Write(static_cast<uint16_t>(lobby.size()));
    for (const auto& p : lobby) {
        writer.WriteLobbyPlayer(p);
    }

    BroadcastPacket(writer, true);
}

std::vector<LobbyPlayerInfo> NetworkServer::GetLobbyPlayers() const {
    std::vector<LobbyPlayerInfo> list;
    if (role_ != ServerRole::Dedicated) {
        list.push_back({1, hostName_.empty() ? "Host" : hostName_, true, true});
    }

    for (const auto& [peer, info] : clients_) {
        if (info.joined) {
            list.push_back({info.playerId, info.name, false, true});
        }
    }
    return list;
}

void NetworkServer::ResetRoomToLobby() {
    isGameStarted_ = false;
    turnNumber_ = 0;
    pendingIntents_.clear();
    lobbyTimer_ = 0.0f;
    gameOverTimer_ = 0.0f;
}

void NetworkServer::SendFullSync(ENetPeer* peer, const Simulation& sim) {
    if (!peer || !udpServer_.IsRunning()) {
        return;
    }

    ByteWriter writer;
    writer.Write(static_cast<uint8_t>(PacketType::FullSync));
    writer.Write(turnNumber_);
    writer.Write(sim.GetMatchTime());
    writer.Write(seed_);
    writer.Write(nations_);
    writer.Write(bots_);
    writer.Write(static_cast<uint8_t>(mapType_));

    // Players
    const auto& players = sim.GetPlayers();
    writer.Write(static_cast<uint16_t>(players.size()));
    for (const auto& p : players) {
        writer.Write(p.id);
        writer.WriteString(p.name);
        writer.Write(static_cast<uint8_t>(p.type));
        writer.Write(p.troops);
        writer.Write(p.gold);
        writer.Write(p.attackRatio);
        writer.Write(p.isAlive);
        writer.Write(p.color);
    }

    // Buildings
    const auto& buildings = sim.GetBuildings();
    writer.Write(static_cast<uint16_t>(buildings.size()));
    for (const auto& b : buildings) {
        writer.Write(static_cast<uint8_t>(b.type));
        writer.Write(b.ownerId);
        writer.Write(b.x);
        writer.Write(b.y);
        writer.Write(b.level);
        writer.Write(b.active);
    }

    // Tile owners RLE stream
    const WorldMap& map = sim.GetMap();
    int totalTiles = map.GetWidth() * map.GetHeight();

    std::vector<std::pair<uint16_t, uint16_t>> runs;
    if (totalTiles > 0) {
        uint16_t currentOwner = map.GetTile(0, 0).ownerId;
        uint16_t currentLen = 1;
        for (int i = 1; i < totalTiles; ++i) {
            int tx = i % map.GetWidth();
            int ty = i / map.GetWidth();
            uint16_t o = map.GetTile(tx, ty).ownerId;
            if (o == currentOwner && currentLen < 65535) {
                currentLen++;
            } else {
                runs.push_back({currentLen, currentOwner});
                currentOwner = o;
                currentLen = 1;
            }
        }
        runs.push_back({currentLen, currentOwner});
    }

    writer.Write(static_cast<uint16_t>(runs.size()));
    for (const auto& [len, owner] : runs) {
        writer.Write(len);
        writer.Write(owner);
    }

    SendPacket(peer, writer, true);
}

void NetworkServer::SendPacket(ENetPeer* peer, const ByteWriter& writer, bool reliable) {
    udpServer_.SendPacket(peer, writer, reliable);
}

void NetworkServer::BroadcastPacket(const ByteWriter& writer, bool reliable) {
    udpServer_.BroadcastPacket(writer, reliable);
}

} // namespace Openfront
