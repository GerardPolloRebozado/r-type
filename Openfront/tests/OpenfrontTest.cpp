#include "Building.hpp"
#include "Combat.hpp"
#include "Common.hpp"
#include "Map.hpp"
#include "NetworkClient.hpp"
#include "NetworkManager.hpp"
#include "NetworkProtocol.hpp"
#include "NetworkServer.hpp"
#include "Player.hpp"
#include "Simulation.hpp"
#include <cassert>
#include <chrono>
#include <cmath>
#include <iostream>
#include <thread>

using namespace Openfront;

void TestMapGeneration() {
    std::cout << "[TEST] Running TestMapGeneration..." << std::endl;
    int mapW = 0, mapH = 0;
    WorldMap::MapSizeFor(MapType::Procedural, mapW, mapH);
    WorldMap map(mapW, mapH);
    map.Generate(42);

    assert(map.GetWidth() == mapW);
    assert(map.GetHeight() == mapH);
    assert(map.GetTotalTiles() == mapW * mapH);
    assert(mapW == 900 && mapH == 562);

    int landCount = map.GetLandTilesCount();
    assert(landCount > 80000);
    assert(landCount < 620000);

    // Verify terrain diversity
    int plains = 0, highlands = 0, mountains = 0, water = 0;
    for (int y = 0; y < map.GetHeight(); ++y) {
        for (int x = 0; x < map.GetWidth(); ++x) {
            switch (map.GetTile(x, y).terrain) {
                case TerrainType::Plains:
                    plains++;
                    break;
                case TerrainType::Highlands:
                    highlands++;
                    break;
                case TerrainType::Mountains:
                    mountains++;
                    break;
                case TerrainType::Water:
                    water++;
                    break;
            }
        }
    }

    assert(plains > 0);
    assert(highlands > 0);
    assert(mountains > 0);
    assert(water > 0);

    std::cout << "  Land count: " << landCount << " / " << mapW * mapH << " (Plains: " << plains
              << ", Highlands: " << highlands << ", Mountains: " << mountains
              << ", Water: " << water << ")" << std::endl;
    std::cout << "[PASS] TestMapGeneration passed." << std::endl;
}

void TestEconomyFormulas() {
    std::cout << "[TEST] Running TestEconomyFormulas..." << std::endl;
    Player p;
    p.id = 1;
    p.tileCount = 100;
    p.cityLevelsSum = 3;
    p.troops = 40000.0;
    p.gold = 10000.0;

    p.CalculateMaxTroops();

    // basePop = (100)^0.6 * 1000 + 50000 = ~65848.9
    // cityBonus = 3 * 25000 = 75000
    // max = 2 * basePop + cityBonus = ~206697.8
    double basePop = std::pow(100.0, 0.6) * 1000.0 + 50000.0;
    double expectedMax = 2.0 * basePop + 75000.0;

    assert(std::abs(p.maxTroops - expectedMax) < 1.0);

    // Troop density & shield
    double density = p.GetTroopDensity();
    assert(std::abs(density - (40000.0 / 100.0)) < 1e-4);
    assert(std::abs(p.GetShield() - (density * 10.0)) < 1e-4);

    // Economy tick (1.0 second)
    p.portCount = 2;
    p.factoryCount = 1;
    double initialGold = p.gold;
    p.TickEconomy(1.0f);

    // Base gold: 1000 + 2*600 + 1*800 = 3000/s
    double expectedGold = initialGold + 3000.0;
    assert(std::abs(p.gold - expectedGold) < 1.0);
    assert(p.troops > 40000.0); // Troops grew towards max

    std::cout << "[PASS] TestEconomyFormulas passed." << std::endl;
}

void TestSimulationCombatAndExpansion() {
    std::cout << "[TEST] Running TestSimulationCombatAndExpansion..." << std::endl;
    Simulation sim;
    sim.Init(2, 2, 777);

    assert(sim.GetPlayers().size() == 5); // 1 human + 2 nations + 2 bots

    Player* human = sim.GetPlayer(1);
    assert(human != nullptr);
    assert(human->isAlive);
    assert(human->tileCount > 0);

    // Find a border tile owned by human and an adjacent neutral tile
    WorldMap& map = sim.GetMap();
    int startX = -1, startY = -1;
    int targetX = -1, targetY = -1;

    for (int y = 0; y < map.GetHeight() && startX < 0; ++y) {
        for (int x = 0; x < map.GetWidth(); ++x) {
            if (map.GetTile(x, y).ownerId == 1 && map.GetTile(x, y).isBorder) {
                auto nbrs = map.GetNeighbors4(x, y);
                for (const auto& nb : nbrs) {
                    if (map.GetTile(nb.x, nb.y).terrain != TerrainType::Water &&
                        map.GetTile(nb.x, nb.y).ownerId != 1) {
                        startX = x;
                        startY = y;
                        targetX = nb.x;
                        targetY = nb.y;
                        break;
                    }
                }
                if (startX >= 0) {
                    break;
                }
            }
        }
    }

    assert(startX >= 0 && targetX >= 0);

    int initialTiles = human->tileCount;
    bool launched = sim.LaunchAttack(1, startX, startY, targetX, targetY, 0.25f);
    assert(launched);
    assert(sim.GetAttackFronts().size() == 1);

    // Tick simulation until attack completes
    for (int t = 0; t < 50; ++t) {
        sim.Tick(kSimulationTickDt);
        if (sim.GetAttackFronts().empty()) {
            break;
        }
    }

    // Verify tile conquest
    assert(map.GetTile(targetX, targetY).ownerId == 1);
    assert(human->tileCount > initialTiles);

    std::cout << "[PASS] TestSimulationCombatAndExpansion passed." << std::endl;
}

void TestNuclearMissile() {
    std::cout << "[TEST] Running TestNuclearMissile..." << std::endl;
    Simulation sim;
    sim.Init(1, 1, 999);

    Player* human = sim.GetPlayer(1);
    assert(human != nullptr);

    // Find owned interior tile for Missile Silo
    WorldMap& map = sim.GetMap();
    int siloX = -1, siloY = -1;
    for (int y = 0; y < map.GetHeight() && siloX < 0; ++y) {
        for (int x = 0; x < map.GetWidth(); ++x) {
            if (map.GetTile(x, y).ownerId == 1 &&
                map.GetTile(x, y).building == BuildingType::None) {
                siloX = x;
                siloY = y;
                break;
            }
        }
    }

    assert(siloX >= 0);

    human->gold = 200000.0;
    bool built = sim.Build(1, siloX, siloY, BuildingType::MissileSilo);
    assert(built);
    assert(map.GetTile(siloX, siloY).building == BuildingType::MissileSilo);

    // Find target tile far away
    int targetX = std::clamp(siloX + 15, 0, map.GetWidth() - 1);
    int targetY = std::clamp(siloY + 15, 0, map.GetHeight() - 1);

    bool launched = sim.LaunchNuke(1, siloX, siloY, targetX, targetY, BombType::AtomBomb);
    assert(launched);
    assert(sim.GetMissiles().size() == 1);

    // Advance until explosion
    for (int t = 0; t < 100; ++t) {
        sim.Tick(kSimulationTickDt);
        if (sim.GetMissiles().empty()) {
            break;
        }
    }

    assert(sim.GetMissiles().empty());
    assert(!sim.GetExplosions().empty());

    // Target tile should be neutralized with fallout timer active
    assert(map.GetTile(targetX, targetY).ownerId == kNeutralPlayerId);
    if (map.GetTile(targetX, targetY).terrain != TerrainType::Water) {
        assert(map.GetTile(targetX, targetY).falloutTimer > 0.0f);
    }

    std::cout << "[PASS] TestNuclearMissile passed." << std::endl;
}

void TestDeterministic10HzSimulation() {
    std::cout << "[TEST] Running TestDeterministic10HzSimulation (100 ticks)..." << std::endl;
    Simulation sim;
    sim.Init(4, 4, 12345);

    for (int i = 0; i < 100; ++i) {
        sim.Tick(kSimulationTickDt);
        sim.UpdateVisuals(kSimulationTickDt);
    }

    assert(std::abs(sim.GetMatchTime() - 10.0f) < 1e-3);
    for (const auto& p : sim.GetPlayers()) {
        assert(!std::isnan(p.troops));
        assert(!std::isnan(p.gold));
        assert(!std::isnan(p.maxTroops));
        assert(p.troops >= 0.0);
        assert(p.gold >= 0.0);
    }

    std::cout << "[PASS] TestDeterministic10HzSimulation passed." << std::endl;
}

void TestNetworkIntentSerialization() {
    std::cout << "[TEST] Running TestNetworkIntentSerialization..." << std::endl;

    ClientIntent it1;
    it1.playerId = 2;
    it1.type = IntentType::Attack;
    it1.startX = 25;
    it1.startY = 30;
    it1.targetX = 26;
    it1.targetY = 30;
    it1.ratio = 0.50f;

    ClientIntent it2;
    it2.playerId = 3;
    it2.type = IntentType::Build;
    it2.startX = 40;
    it2.startY = 45;
    it2.buildingType = BuildingType::DefensePost;

    ByteWriter writer;
    writer.Write(static_cast<uint8_t>(PacketType::ServerTurn));
    writer.Write(static_cast<uint32_t>(105)); // Turn 105
    writer.Write(static_cast<uint16_t>(2));   // 2 intents
    writer.WriteIntent(it1);
    writer.WriteIntent(it2);

    assert(writer.Size() > 0);

    ByteReader reader(writer.Data(), writer.Size());
    uint8_t pktType = 0;
    uint32_t turnNumber = 0;
    uint16_t count = 0;

    assert(reader.Read(pktType));
    assert(static_cast<PacketType>(pktType) == PacketType::ServerTurn);
    assert(reader.Read(turnNumber) && turnNumber == 105);
    assert(reader.Read(count) && count == 2);

    ClientIntent out1, out2;
    assert(reader.ReadIntent(out1));
    assert(reader.ReadIntent(out2));

    assert(out1.playerId == 2);
    assert(out1.type == IntentType::Attack);
    assert(out1.startX == 25 && out1.startY == 30);
    assert(out1.targetX == 26 && out1.targetY == 30);
    assert(std::abs(out1.ratio - 0.50f) < 1e-4);

    assert(out2.playerId == 3);
    assert(out2.type == IntentType::Build);
    assert(out2.startX == 40 && out2.startY == 45);
    assert(out2.buildingType == BuildingType::DefensePost);

    std::cout << "[PASS] TestNetworkIntentSerialization passed." << std::endl;
}

void TestMultiplayerLockstepLoopback() {
    std::cout << "[TEST] Running TestMultiplayerLockstepLoopback..." << std::endl;

    Simulation serverSim;
    Simulation clientSim;

    NetworkServer server;
    NetworkClient client;

    uint16_t testPort = 7799;
    serverSim.Init(3, 3, 54321);
    server.SetMatchSettings(54321, 3, 3, 1800.0f);

    bool srvStarted = server.Start(testPort, 8);
    assert(srvStarted);

    bool cliStarted = client.Connect("127.0.0.1", testPort, "RemoteCommander");
    assert(cliStarted);

    // Pump events until client is joined
    for (int i = 0; i < 100 && !client.HasJoined(); ++i) {
        server.PollNetwork(serverSim, 0.01f);
        client.PollNetwork(clientSim, 0.01f);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    assert(client.HasJoined());
    assert(client.GetLocalPlayerId() == 2);

    // Host starts match with all joined lobby players
    server.StartMatch(serverSim);

    for (int i = 0; i < 50 && !client.IsInGame(); ++i) {
        server.PollNetwork(serverSim, 0.01f);
        client.PollNetwork(clientSim, 0.01f);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    assert(client.IsInGame());
    assert(serverSim.GetPlayer(2) != nullptr);
    assert(serverSim.GetPlayer(2)->name == "RemoteCommander");
    assert(clientSim.GetPlayer(2) != nullptr);
    assert(clientSim.GetPlayer(2)->name == "RemoteCommander");

    // Client queues an attack intent
    ClientIntent testAttack;
    testAttack.playerId = 2;
    testAttack.type = IntentType::Attack;
    testAttack.startX = 50;
    testAttack.startY = 50;
    testAttack.targetX = 51;
    testAttack.targetY = 50;
    testAttack.ratio = 0.20f;
    client.SendIntent(testAttack);

    // Pump packets so server receives the intent
    for (int i = 0; i < 20; ++i) {
        server.PollNetwork(serverSim, 0.01f);
        client.PollNetwork(clientSim, 0.01f);
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    // Server executes turn 0 and broadcasts to client
    server.TickTurn(serverSim);

    // Pump packets so client receives turn 0
    for (int i = 0; i < 20; ++i) {
        server.PollNetwork(serverSim, 0.01f);
        client.PollNetwork(clientSim, 0.01f);
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    assert(client.GetCurrentTurn() == 0);
    assert(serverSim.GetCurrentTurn() == 1); // Server completed turn 0 and is now at 1

    client.Disconnect();
    server.Stop();

    std::cout << "[PASS] TestMultiplayerLockstepLoopback passed." << std::endl;
}

void TestNoBotExecutionOnHumanPlayers() {
    std::cout << "[TEST] Running TestNoBotExecutionOnHumanPlayers..." << std::endl;

    Simulation sim;
    sim.Init(4, 4, 9999);

    // Player 1 is Human by default
    // Assign Player 2 as Human
    sim.AssignPlayerSlot(2, "Alex", FactionType::Human);

    const Player* p1 = sim.GetPlayer(1);
    const Player* p2 = sim.GetPlayer(2);
    assert(p1 && p1->type == FactionType::Human);
    assert(p2 && p2->type == FactionType::Human);

    // Simulate 50 ticks of AI intent collection
    for (int tick = 0; tick < 50; ++tick) {
        std::vector<ClientIntent> aiIntents;
        sim.CollectAIIntents(aiIntents, kSimulationTickDt);

        for (const auto& intent : aiIntents) {
            // Under NO circumstance should any AI intent be generated for Human Players 1 or 2!
            assert(intent.playerId != 1);
            assert(intent.playerId != 2);
        }

        sim.ExecuteTurn(aiIntents, kSimulationTickDt);
    }

    // Verify Player 1 and Player 2 did NOT execute any autonomous attacks
    for (const auto& front : sim.GetAttackFronts()) {
        assert(front.attackerId != 1);
        assert(front.attackerId != 2);
    }

    std::cout << "[PASS] TestNoBotExecutionOnHumanPlayers passed." << std::endl;
}

void TestNavalInvasion() {
    std::cout << "[TEST] Running TestNavalInvasion..." << std::endl;
    Simulation sim;
    sim.Init(2, 2, 4242);

    WorldMap& map = sim.GetMap();
    const int W = map.GetWidth();
    const int H = map.GetHeight();
    Player* human = sim.GetPlayer(1);
    Player* enemy = sim.GetPlayer(2);
    assert(human && enemy);

    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            Tile& t = map.GetTile(x, y);
            bool islA = (x >= 8 && x <= 12 && y >= 58 && y <= 62);
            bool islB = (x >= 18 && x <= 22 && y >= 58 && y <= 62);
            t.terrain = (islA || islB) ? TerrainType::Plains : TerrainType::Water;
            t.ownerId = islA ? 1 : (islB ? 2 : kNeutralPlayerId);
            t.building = BuildingType::None;
            t.buildingLevel = 0;
            t.hasDefenseBonus = false;
            t.captureFlashTimer = 0.0f;
            t.falloutTimer = 0.0f;
        }
    }
    map.UpdateBorders();
    map.RebuildCoastalWater();
    map.RecountOwnerTiles();

    human->troops = 10000.0;
    human->tileCount = 25;
    enemy->tileCount = 25;
    for (PlayerId id = 2; id <= 5; ++id) {
        Player* p = sim.GetPlayer(id);
        if (p) {
            p->troops = 0.0;
        }
    }

    bool launched = sim.LaunchAttack(1, 20, 58, 1.0f);
    assert(launched);

    const FrontPush* naval = nullptr;
    for (const auto& f : sim.GetAttackFronts()) {
        if (f.targetX == 20 && f.targetY == 58) {
            naval = &f;
        }
    }
    assert(naval != nullptr);
    assert(naval->naval);

    bool landed = false;
    for (int ticks = 0; ticks < 300 && !landed; ++ticks) {
        sim.Tick(kSimulationTickDt);
        landed = (map.GetTile(20, 58).ownerId == 1);
    }
    assert(landed);

    assert(map.GetTile(13, 58).ownerId == kNeutralPlayerId);
    assert(map.GetTile(16, 58).ownerId == kNeutralPlayerId);
    assert(map.GetTile(18, 58).ownerId == 1);
    assert(human->tileCount > 25);

    std::cout << "[PASS] TestNavalInvasion passed." << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "      OPENFRONT SIMULATION TEST SUITE   " << std::endl;
    std::cout << "========================================" << std::endl;

    TestMapGeneration();
    TestEconomyFormulas();
    TestSimulationCombatAndExpansion();
    TestNavalInvasion();
    TestNuclearMissile();
    TestDeterministic10HzSimulation();
    TestNetworkIntentSerialization();
    TestMultiplayerLockstepLoopback();
    TestNoBotExecutionOnHumanPlayers();

    std::cout << "========================================" << std::endl;
    std::cout << "      ALL OPENFRONT TESTS PASSED!       " << std::endl;
    std::cout << "========================================" << std::endl;
    return 0;
}
