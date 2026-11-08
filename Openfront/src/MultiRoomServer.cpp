#include "MultiRoomServer.hpp"
#include "Logging/Logger.hpp"
#include <chrono>
#include <thread>

namespace Openfront {

bool MultiRoomServer::Start(const std::vector<RoomConfig>& configs) {
    if (rooms_.empty()) {
        configs_ = configs;
        for (const auto& cfg : configs) {
            auto room = std::make_unique<Room>();
            room->cfg = cfg;

            room->sim.Init(4, cfg.bots, cfg.seed, cfg.mapType);

            room->server.ConfigureRoom(cfg.name, cfg.maxPlayers, cfg.bots, cfg.mapType,
                                       cfg.autoStartDelay);
            room->server.SetMatchSettings(cfg.seed, cfg.maxPlayers, cfg.bots,
                                          room->sim.GetMatchDuration());

            if (!room->server.Start(cfg.port, cfg.maxPlayers,
                                    NetworkServer::ServerRole::Dedicated)) {
                log_error("[MultiRoomServer] Failed to bind '" + cfg.name + "' on port " +
                          std::to_string(cfg.port));
                Stop();
                return false;
            }
            rooms_.push_back(std::move(room));

            log_info("[MultiRoomServer] Room '" + cfg.name + "' up: port " +
                     std::to_string(cfg.port) + ", " + std::to_string(cfg.maxPlayers) +
                     " human slots, " + std::to_string(cfg.bots) + " bots, map " +
                     MapTypeName(cfg.mapType));
        }
    }
    running_ = true;
    return true;
}

void MultiRoomServer::Run() {
    auto lastTick = std::chrono::steady_clock::now();

    while (running_) {
        auto now = std::chrono::steady_clock::now();
        float dt = std::chrono::duration<float>(now - lastTick).count();
        if (dt > 0.25f) {
            dt = 0.25f;
        }
        lastTick = now;

        for (auto& room : rooms_) {
            if (!room->server.IsRunning()) {
                continue;
            }

            room->server.PollNetwork(room->sim, dt);

            if (room->server.IsGameStarted()) {
                room->accumulator += dt;
                while (room->accumulator >= kSimulationTickDt) {
                    room->server.TickTurn(room->sim);
                    room->accumulator -= kSimulationTickDt;
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
}

void MultiRoomServer::Stop() {
    running_ = false;
    for (auto& room : rooms_) {
        room->server.Stop();
    }
    rooms_.clear();
}

} // namespace Openfront
