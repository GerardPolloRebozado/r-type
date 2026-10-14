#include "Components/Position.hpp"
#include "ECS/Registry.hpp"
#include "Resources/NetworkHostResource.hpp"
#include "Resources/PacketRouterResource.hpp"
#include <cassert>
#include <iostream>

struct KeyPress {
    char key = 0;
};

struct MoveTo {
    std::size_t id;
    Engine::Position pos;
};

static int failures = 0;
#define CHECK(cond)                                                                                \
    do {                                                                                           \
        if (!(cond)) {                                                                             \
            std::cerr << "FAIL line " << __LINE__ << ": " #cond "\n";                              \
            ++failures;                                                                            \
        }                                                                                          \
    } while (0)

int main() {
    const PacketTypeID keyId = Engine::Multiplayer::GetPacketTypeID<KeyPress>();
    const PacketTypeID moveId = Engine::Multiplayer::GetPacketTypeID<MoveTo>();

    Engine::Registry registry;
    Engine::Multiplayer::PacketRouterResource router([](Engine::Registry&, ENetPeer*) {},
                                                     [](Engine::Registry&, ENetPeer*) {});

    char receivedKey = 0;
    int keyCalls = 0;
    router.RegisterHandler<KeyPress>(keyId, [&](Engine::Registry&, ENetPeer*, const KeyPress& kp) {
        receivedKey = kp.key;
        ++keyCalls;
    });

    MoveTo receivedMove{};
    int moveCalls = 0;
    router.RegisterHandler<MoveTo>(moveId, [&](Engine::Registry&, ENetPeer*, const MoveTo& m) {
        receivedMove = m;
        ++moveCalls;
    });

    // Valid packets
    ENetPacket* p1 =
        Engine::Multiplayer::NetworkHostResource::CreatePacket(keyId, KeyPress{'x'}, true);
    router.Dispatch(registry, nullptr, p1->data, p1->dataLength);
    enet_packet_destroy(p1);
    CHECK(keyCalls == 1);
    CHECK(receivedKey == 'x');

    MoveTo sent{7, Engine::Position::Create(1.5f, -2.5f)};
    ENetPacket* p2 = Engine::Multiplayer::NetworkHostResource::CreatePacket(moveId, sent, false);
    router.Dispatch(registry, nullptr, p2->data, p2->dataLength);
    enet_packet_destroy(p2);
    CHECK(moveCalls == 1);
    CHECK(receivedMove.id == 7);
    CHECK(receivedMove.pos.x == 1.5f);
    CHECK(receivedMove.pos.y == -2.5f);

    // Unknown packet id -> no handler invoked
    ENetPacket* p3 = Engine::Multiplayer::NetworkHostResource::CreatePacket(
        static_cast<PacketTypeID>(999), KeyPress{'y'}, true);
    router.Dispatch(registry, nullptr, p3->data, p3->dataLength);
    enet_packet_destroy(p3);
    CHECK(keyCalls == 1);
    CHECK(moveCalls == 1);

    // Truncated / garbage input must not crash nor invoke handlers
    router.Dispatch(registry, nullptr, nullptr, 0);
    const uint8_t oneByte[1] = {0xFF};
    router.Dispatch(registry, nullptr, oneByte, 1);
    CHECK(keyCalls == 1);
    CHECK(moveCalls == 1);

    if (failures == 0) {
        std::cout << "ALL PACKET CHECKS PASSED\n";
        return 0;
    }
    std::cerr << failures << " PACKET CHECKS FAILED\n";
    return 1;
}
