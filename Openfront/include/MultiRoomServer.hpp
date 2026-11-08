#ifndef OPENFRONT_MULTIROOM_SERVER_HPP_
#define OPENFRONT_MULTIROOM_SERVER_HPP_

#include "NetworkProtocol.hpp"
#include "NetworkServer.hpp"
#include "Simulation.hpp"
#include <memory>
#include <string>
#include <vector>

namespace Openfront {

struct RoomConfig {
    std::string name{"Room"};
    uint16_t port{0};
    uint16_t maxPlayers{8};
    uint8_t bots{6};
    MapType mapType{MapType::Procedural};
    float autoStartDelay{8.0f};
    uint32_t seed{1337};
};

class MultiRoomServer {
  public:
    bool Start(const std::vector<RoomConfig>& configs);
    void Run();
    void Stop();

    [[nodiscard]] const std::vector<RoomConfig>& GetRoomConfigs() const {
        return configs_;
    }

  private:
    struct Room {
        RoomConfig cfg;
        NetworkServer server;
        Simulation sim;
        float accumulator{0.0f};
    };

    std::vector<std::unique_ptr<Room>> rooms_;
    std::vector<RoomConfig> configs_;
    bool running_{false};
};

} // namespace Openfront

#endif
