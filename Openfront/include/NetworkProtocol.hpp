#ifndef OPENFRONT_NETWORK_PROTOCOL_HPP_
#define OPENFRONT_NETWORK_PROTOCOL_HPP_

#include "Common.hpp"
#include "Map.hpp"
#include "Network/ByteStream.hpp"
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace Openfront {

constexpr uint16_t kDefaultPort = 7777;
constexpr uint8_t kNetworkProtocolVersion = 1;

enum class PacketType : uint8_t {
    JoinRequest = 1,
    JoinResponse = 2,
    ClientIntent = 3,
    ServerTurn = 4,
    PlayerJoined = 5,
    PlayerLeft = 6,
    FullSync = 7,
    Ping = 8,
    Pong = 9,
    LobbyState = 10,
    StartGame = 11,
    RoomInfo = 12
};

inline const char* MapTypeName(MapType type) {
    switch (type) {
        case MapType::Europe:
            return "Europe";
        case MapType::World:
            return "World";
        case MapType::Pangaea:
            return "Pangaea";
        case MapType::Archipelago:
            return "Archipelago";
        default:
            return "Procedural";
    }
}

inline bool ParseMapTypeName(const std::string& name, MapType& out) {
    if (name == "procedural" || name == "Procedural") {
        out = MapType::Procedural;
    } else if (name == "europe" || name == "Europe") {
        out = MapType::Europe;
    } else if (name == "world" || name == "World") {
        out = MapType::World;
    } else if (name == "pangaea" || name == "Pangaea") {
        out = MapType::Pangaea;
    } else if (name == "archipelago" || name == "Archipelago") {
        out = MapType::Archipelago;
    } else {
        return false;
    }
    return true;
}

struct RoomInfo {
    std::string name;
    MapType mapType{MapType::Procedural};
    uint16_t players{0};
    uint16_t maxPlayers{0};
    bool gameStarted{false};
    uint32_t turn{0};
};

enum class IntentType : uint8_t {
    Attack = 1,
    Reinforce = 2,
    Build = 3,
    LaunchNuke = 4,
    SetAttackRatio = 5
};

struct ClientIntent {
    PlayerId playerId{kNeutralPlayerId};
    IntentType type{IntentType::Attack};
    int startX{0};
    int startY{0};
    int targetX{0};
    int targetY{0};
    uint32_t frontId{0};
    BuildingType buildingType{BuildingType::None};
    BombType bombType{BombType::AtomBomb};
    float ratio{0.20f};
};

struct LobbyPlayerInfo {
    PlayerId id{kNeutralPlayerId};
    std::string name;
    bool isHost{false};
    bool isReady{true};
};

// Specialized Binary Buffer Writer derived from Engine::Network::ByteWriter
class ByteWriter : public Engine::Network::ByteWriter {
  public:
    using Engine::Network::ByteWriter::ByteWriter;

    void WriteIntent(const ClientIntent& intent) {
        Write(intent.playerId);
        Write(static_cast<uint8_t>(intent.type));
        Write(intent.startX);
        Write(intent.startY);
        Write(intent.targetX);
        Write(intent.targetY);
        Write(intent.frontId);
        Write(static_cast<uint8_t>(intent.buildingType));
        Write(static_cast<uint8_t>(intent.bombType));
        Write(intent.ratio);
    }

    void WriteLobbyPlayer(const LobbyPlayerInfo& p) {
        Write(p.id);
        WriteString(p.name);
        Write(p.isHost);
        Write(p.isReady);
    }

    void WriteRoomInfo(const RoomInfo& room) {
        WriteString(room.name);
        Write(static_cast<uint8_t>(room.mapType));
        Write(room.players);
        Write(room.maxPlayers);
        Write(room.gameStarted);
        Write(room.turn);
    }
};

// Specialized Binary Buffer Reader derived from Engine::Network::ByteReader
class ByteReader : public Engine::Network::ByteReader {
  public:
    using Engine::Network::ByteReader::ByteReader;

    bool ReadIntent(ClientIntent& out) {
        uint8_t type = 0, bType = 0, nType = 0;
        if (!Read(out.playerId) || !Read(type) || !Read(out.startX) || !Read(out.startY) ||
            !Read(out.targetX) || !Read(out.targetY) || !Read(out.frontId) || !Read(bType) ||
            !Read(nType) || !Read(out.ratio)) {
            return false;
        }

        out.type = static_cast<IntentType>(type);
        out.buildingType = static_cast<BuildingType>(bType);
        out.bombType = static_cast<BombType>(nType);
        return true;
    }

    bool ReadLobbyPlayer(LobbyPlayerInfo& out) {
        return Read(out.id) && ReadString(out.name) && Read(out.isHost) && Read(out.isReady);
    }

    bool ReadRoomInfo(RoomInfo& out) {
        uint8_t mapType = 0;
        if (!ReadString(out.name) || !Read(mapType) || !Read(out.players) ||
            !Read(out.maxPlayers) || !Read(out.gameStarted) || !Read(out.turn)) {
            return false;
        }
        out.mapType = static_cast<MapType>(mapType);
        return true;
    }
};

// Protocol Packets
struct MsgJoinRequest {
    std::string playerName;
};

struct MsgJoinResponse {
    bool accepted{true};
    PlayerId assignedPlayerId{kNeutralPlayerId};
    uint32_t mapSeed{1337};
    uint8_t numNations{4};
    uint8_t numBots{4};
    float matchDuration{1800.0f};
    uint32_t currentTurn{0};
};

struct MsgServerTurn {
    uint32_t turnNumber{0};
    std::vector<ClientIntent> intents;
};

struct MsgPlayerJoined {
    PlayerId id{kNeutralPlayerId};
    std::string name;
    Color color{WHITE};
};

struct MsgPlayerLeft {
    PlayerId id{kNeutralPlayerId};
};

struct MsgLobbyState {
    uint32_t mapSeed{1337};
    uint8_t numNations{4};
    uint8_t numBots{4};
    std::vector<LobbyPlayerInfo> players;
};

struct MsgStartGame {
    uint32_t mapSeed{1337};
    uint8_t numNations{4};
    uint8_t numBots{4};
    float matchDuration{1800.0f};
    std::vector<LobbyPlayerInfo> players;
};

} // namespace Openfront

#endif // OPENFRONT_NETWORK_PROTOCOL_HPP_
