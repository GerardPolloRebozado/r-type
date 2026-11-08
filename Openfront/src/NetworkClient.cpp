#include "NetworkClient.hpp"
#include "Logging/Logger.hpp"
#include <chrono>
#include <iostream>
#include <thread>

namespace Openfront {

NetworkClient::NetworkClient() = default;

NetworkClient::~NetworkClient() {
    Disconnect();
}

bool NetworkClient::Connect(const std::string& hostAddress, uint16_t port,
                            const std::string& playerName) {
    if (udpClient_.IsConnected()) {
        Disconnect();
    }

    if (!udpClient_.Connect(hostAddress, port, 2)) {
        log_error("[NetworkClient] Failed to initiate connection to " + hostAddress + ":" +
                  std::to_string(port));
        return false;
    }

    playerName_ = playerName;
    state_ = ClientState::Connecting;
    currentTurn_ = 0;
    pingTimer_ = 0.0f;

    log_info("[NetworkClient] Connecting to " + hostAddress + ":" + std::to_string(port) + " as '" +
             playerName + "'...");
    return true;
}

void NetworkClient::Disconnect() {
    udpClient_.Disconnect();
    state_ = ClientState::Disconnected;
    localPlayerId_ = kNeutralPlayerId;
}

uint32_t NetworkClient::GetPing() const {
    return udpClient_.GetPing();
}

void NetworkClient::SendIntent(const ClientIntent& intent) {
    if (state_ != ClientState::InGame || !udpClient_.IsConnected()) {
        return;
    }

    ByteWriter writer;
    writer.Write(static_cast<uint8_t>(PacketType::ClientIntent));
    writer.WriteIntent(intent);
    SendPacket(writer, true);
}

void NetworkClient::PollNetwork(Simulation& sim, float dt) {
    if (!udpClient_.IsConnected() && state_ == ClientState::Disconnected) {
        return;
    }

    pingTimer_ += dt;
    if (pingTimer_ >= 2.0f && udpClient_.IsConnected()) {
        pingTimer_ = 0.0f;
        ByteWriter ping;
        ping.Write(static_cast<uint8_t>(PacketType::Ping));
        SendPacket(ping, false);
    }

    udpClient_.PollEvents([&](const Engine::Network::NetworkEvent& event) {
        switch (event.type) {
            case Engine::Network::NetworkEventType::Connect: {
                state_ = ClientState::Connected;
                log_info("[NetworkClient] Connected to server! Sending JoinRequest...");

                // Send JoinRequest
                ByteWriter req;
                req.Write(static_cast<uint8_t>(PacketType::JoinRequest));
                req.WriteString(playerName_);
                SendPacket(req, true);
                break;
            }
            case Engine::Network::NetworkEventType::Receive: {
                HandlePacket(sim, event.data.data(), event.data.size());
                break;
            }
            case Engine::Network::NetworkEventType::Disconnect: {
                log_info("[NetworkClient] Disconnected from server.");
                state_ = ClientState::Disconnected;
                if (onDisconnected_) {
                    onDisconnected_();
                }
                break;
            }
            default:
                break;
        }
    });
}

void NetworkClient::HandlePacket(Simulation& sim, const uint8_t* data, size_t size) {
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
        case PacketType::JoinResponse: {
            bool accepted = false;
            PlayerId assignedId = 0;
            uint32_t seed = 0;
            uint8_t nations = 0;
            uint8_t bots = 0;
            float duration = 1800.0f;
            uint32_t srvTurn = 0;
            uint8_t mapType = 0;

            if (!reader.Read(accepted) || !accepted) {
                log_error("[NetworkClient] Join rejected by server.");
                Disconnect();
                return;
            }

            reader.Read(assignedId);
            reader.Read(seed);
            reader.Read(nations);
            reader.Read(bots);
            reader.Read(duration);
            reader.Read(srvTurn);
            reader.Read(mapType);

            localPlayerId_ = assignedId;
            currentTurn_ = srvTurn;
            serverMapType_ = static_cast<MapType>(mapType);
            state_ = ClientState::InLobby;

            log_info("[NetworkClient] Joined lobby! Assigned PlayerId " +
                     std::to_string(assignedId));

            if (onJoined_) {
                onJoined_();
            }
            break;
        }
        case PacketType::LobbyState: {
            uint32_t seed = 0;
            uint8_t nations = 0;
            uint8_t bots = 0;
            uint16_t count = 0;
            if (!reader.Read(seed) || !reader.Read(nations) || !reader.Read(bots) ||
                !reader.Read(count)) {
                return;
            }

            lobbyPlayers_.clear();
            for (uint16_t i = 0; i < count; ++i) {
                LobbyPlayerInfo p;
                if (reader.ReadLobbyPlayer(p)) {
                    lobbyPlayers_.push_back(p);
                }
            }

            state_ = ClientState::InLobby;
            log_info("[NetworkClient] Lobby updated: " + std::to_string(lobbyPlayers_.size()) +
                     " player(s) present.");
            if (onLobbyUpdated_) {
                onLobbyUpdated_();
            }
            break;
        }
        case PacketType::StartGame: {
            uint32_t seed = 0;
            uint8_t nations = 0;
            uint8_t bots = 0;
            float duration = 1800.0f;
            uint8_t mapType = 0;
            uint16_t count = 0;
            if (!reader.Read(seed) || !reader.Read(nations) || !reader.Read(bots) ||
                !reader.Read(duration) || !reader.Read(mapType) || !reader.Read(count)) {
                return;
            }

            lobbyPlayers_.clear();
            for (uint16_t i = 0; i < count; ++i) {
                LobbyPlayerInfo p;
                if (reader.ReadLobbyPlayer(p)) {
                    lobbyPlayers_.push_back(p);
                }
            }

            sim.Init(nations, bots, seed, static_cast<MapType>(mapType));
            for (const auto& p : lobbyPlayers_) {
                sim.AssignPlayerSlot(p.id, p.name, FactionType::Human);
            }
            sim.SetCurrentTurn(0);
            currentTurn_ = 0;
            state_ = ClientState::InGame;

            log_info("[NetworkClient] Match started! Initialized simulation for " +
                     std::to_string(lobbyPlayers_.size()) + " players.");
            if (onGameStarted_) {
                onGameStarted_();
            }
            break;
        }
        case PacketType::FullSync: {
            uint32_t srvTurn = 0;
            float matchTime = 0.0f;
            uint32_t seed = 0;
            uint8_t nations = 0;
            uint8_t bots = 0;
            uint8_t mapType = 0;

            if (!reader.Read(srvTurn) || !reader.Read(matchTime) || !reader.Read(seed) ||
                !reader.Read(nations) || !reader.Read(bots) || !reader.Read(mapType)) {
                return;
            }

            sim.Init(nations, bots, seed, static_cast<MapType>(mapType));
            sim.SetCurrentTurn(srvTurn);
            currentTurn_ = srvTurn;

            // Unpack players
            uint16_t pCount = 0;
            if (!reader.Read(pCount)) {
                return;
            }
            for (uint16_t i = 0; i < pCount; ++i) {
                PlayerId pid;
                std::string pname;
                uint8_t ptype;
                double ptroops, pgold;
                float pratio;
                bool palive;
                Color pcol;

                if (!reader.Read(pid) || !reader.ReadString(pname) || !reader.Read(ptype) ||
                    !reader.Read(ptroops) || !reader.Read(pgold) || !reader.Read(pratio) ||
                    !reader.Read(palive) || !reader.Read(pcol)) {
                    return;
                }

                Player* p = sim.GetPlayer(pid);
                if (p) {
                    p->name = pname;
                    p->type = static_cast<FactionType>(ptype);
                    p->troops = ptroops;
                    p->gold = pgold;
                    p->attackRatio = pratio;
                    p->isAlive = palive;
                    p->color = pcol;
                }
            }

            // Unpack buildings
            uint16_t bCount = 0;
            if (!reader.Read(bCount)) {
                return;
            }
            auto& bList = sim.GetBuildings();
            bList.clear();
            for (uint16_t i = 0; i < bCount; ++i) {
                uint8_t btype;
                PlayerId bowner;
                int32_t bx, by, blevel;
                bool bactive;

                if (!reader.Read(btype) || !reader.Read(bowner) || !reader.Read(bx) ||
                    !reader.Read(by) || !reader.Read(blevel) || !reader.Read(bactive)) {
                    return;
                }

                Building b;
                b.type = static_cast<BuildingType>(btype);
                b.ownerId = bowner;
                b.x = bx;
                b.y = by;
                b.level = blevel;
                b.active = bactive;
                bList.push_back(b);
            }

            // Unpack tile owners
            uint16_t runCount = 0;
            if (!reader.Read(runCount)) {
                return;
            }
            WorldMap& map = sim.GetMap();
            int curIdx = 0;
            int totalTiles = map.GetWidth() * map.GetHeight();

            for (uint16_t i = 0; i < runCount; ++i) {
                uint16_t rLen = 0, rOwner = 0;
                if (!reader.Read(rLen) || !reader.Read(rOwner)) {
                    return;
                }
                for (uint16_t r = 0; r < rLen && curIdx < totalTiles; ++r, ++curIdx) {
                    int tx = curIdx % map.GetWidth();
                    int ty = curIdx / map.GetWidth();
                    map.GetTile(tx, ty).ownerId = rOwner;
                }
            }

            map.UpdateBorders();
            sim.RecalculateTerritoryAndBuildings();

            state_ = ClientState::InGame;
            log_info("[NetworkClient] FullSync received! Synced to Turn " +
                     std::to_string(srvTurn));
            if (onGameStarted_) {
                onGameStarted_();
            }
            break;
        }
        case PacketType::ServerTurn: {
            uint32_t turnNumber = 0;
            uint16_t intentCount = 0;
            if (!reader.Read(turnNumber)) {
                return;
            }
            if (!reader.Read(intentCount)) {
                return;
            }

            std::vector<ClientIntent> intents;
            intents.reserve(intentCount);
            for (uint16_t i = 0; i < intentCount; ++i) {
                ClientIntent it;
                if (reader.ReadIntent(it)) {
                    intents.push_back(it);
                }
            }

            sim.ExecuteTurn(intents, kSimulationTickDt);
            currentTurn_ = turnNumber;
            break;
        }
        case PacketType::PlayerJoined: {
            PlayerId pid = 0;
            std::string pname;
            Color pcolor = WHITE;
            if (reader.Read(pid) && reader.ReadString(pname) && reader.Read(pcolor)) {
                sim.AssignPlayerSlot(pid, pname, FactionType::Human);
                log_info("[NetworkClient] Player " + std::to_string(pid) + " ('" + pname +
                         "') joined the match.");
            }
            break;
        }
        case PacketType::PlayerLeft: {
            PlayerId pid = 0;
            if (reader.Read(pid)) {
                Player* p = sim.GetPlayer(pid);
                if (p) {
                    p->type = FactionType::Bot;
                    log_info("[NetworkClient] Player " + std::to_string(pid) +
                             " left. Replaced by Bot AI.");
                }
            }
            break;
        }
        case PacketType::Pong: {
            break;
        }
        default:
            break;
    }
}

void NetworkClient::SendPacket(const ByteWriter& writer, bool reliable) {
    udpClient_.SendPacket(writer, reliable);
}

std::vector<NetworkClient::ScannedRoom> NetworkClient::ScanRooms(const std::string& hostAddress,
                                                                 uint16_t basePort,
                                                                 uint16_t roomCount,
                                                                 float perProbeTimeoutMs) {
    std::vector<ScannedRoom> rooms;
    rooms.reserve(roomCount);

    for (uint16_t i = 0; i < roomCount; ++i) {
        uint16_t port = static_cast<uint16_t>(basePort + i);
        ScannedRoom entry;
        entry.port = port;

        Engine::Network::UdpClient probe;
        if (!probe.Connect(hostAddress, port, 2)) {
            rooms.push_back(entry);
            continue;
        }

        bool gotRoom = false;
        bool probeFailed = false;
        auto deadline = std::chrono::steady_clock::now() +
                        std::chrono::milliseconds(static_cast<long long>(perProbeTimeoutMs));

        while (std::chrono::steady_clock::now() < deadline && !gotRoom && !probeFailed) {
            probe.PollEvents([&](const Engine::Network::NetworkEvent& event) {
                switch (event.type) {
                    case Engine::Network::NetworkEventType::Connect: {
                        ByteWriter req;
                        req.Write(static_cast<uint8_t>(PacketType::RoomInfo));
                        probe.SendPacket(req, true);
                        break;
                    }
                    case Engine::Network::NetworkEventType::Receive: {
                        ByteReader reader(event.data.data(), event.data.size());
                        uint8_t pktType = 0;
                        if (reader.Read(pktType) &&
                            pktType == static_cast<uint8_t>(PacketType::RoomInfo)) {
                            RoomInfo info;
                            if (reader.ReadRoomInfo(info)) {
                                entry.info = info;
                                entry.reachable = true;
                                gotRoom = true;
                            }
                        }
                        break;
                    }
                    case Engine::Network::NetworkEventType::Disconnect: {
                        probeFailed = true;
                        break;
                    }
                    default:
                        break;
                }
            });
            if (gotRoom || probeFailed) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(4));
        }

        if (gotRoom) {
            probe.Disconnect();
        }
        rooms.push_back(entry);
    }

    return rooms;
}

} // namespace Openfront
