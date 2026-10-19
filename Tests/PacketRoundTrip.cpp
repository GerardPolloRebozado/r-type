#include "Components/Position.hpp"
#include "ECS/Registry.hpp"
#include "Resources/NetworkHostResource.hpp"
#include "Resources/PacketRouterResource.hpp"
#include <cassert>
#include <cstddef>
#include <iostream>
#include <span>

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
    Engine::Registry registryA;
    Engine::Registry registryB;
    auto& regA = registryA.AddResource<Engine::Multiplayer::PacketRegistryResource>();
    auto& regB = registryB.AddResource<Engine::Multiplayer::PacketRegistryResource>();

    const PacketTypeID keyIdA = regA.GetOrRegister("key_press");
    const PacketTypeID moveIdA = regA.GetOrRegister("new_player_pos");
    const PacketTypeID moveIdB = regB.GetOrRegister("new_player_pos");
    const PacketTypeID keyIdB = regB.GetOrRegister("key_press");

    CHECK(keyIdA == keyIdB);
    CHECK(moveIdA == moveIdB);
    CHECK(keyIdA != moveIdA);
    CHECK(keyIdA >= Engine::Multiplayer::UserDefinedStart);
    CHECK(regA.TryGet("key_press").value() == keyIdA);
    CHECK(!regA.TryGet("unknown_packet").has_value());
    CHECK(regA.GetName(keyIdA).value() == "key_press");
    CHECK(!regA.GetName(199).has_value()); // reserved range never assigned

    const PacketTypeID keyId = keyIdA;
    const PacketTypeID moveId = moveIdA;

    Engine::Registry registry;
    Engine::Multiplayer::PacketRouterResource router([](Engine::Registry&, ENetPeer*) {},
                                                     [](Engine::Registry&, ENetPeer*) {});

    char receivedKey = 0;
    int keyCalls = 0;
    router.On(keyId, [&](Engine::Registry&, ENetPeer*, const uint8_t* payload, std::size_t size) {
        KeyPress kp{};
        const std::span buffer{reinterpret_cast<const std::byte*>(payload), size};
        if (!glz::read_beve(kp, buffer)) {
            receivedKey = kp.key;
            ++keyCalls;
        }
    });

    MoveTo receivedMove{};
    int moveCalls = 0;
    router.On(moveId, [&](Engine::Registry&, ENetPeer*, const uint8_t* payload, std::size_t size) {
        MoveTo m{};
        const std::span buffer{reinterpret_cast<const std::byte*>(payload), size};
        if (!glz::read_beve(m, buffer)) {
            receivedMove = m;
            ++moveCalls;
        }
    });

    // Raw (Lua-style) handler path.
    int rawCalls = 0;
    const PacketTypeID rawId = regA.GetOrRegister("lua_move");
    router.On(rawId, [&](Engine::Registry&, ENetPeer*, const uint8_t* payload, std::size_t size) {
        ++rawCalls;
        CHECK(size == 3);
        CHECK(payload[0] == 'a');
        (void)payload;
    });
    CHECK(router.HasHandler(rawId));
    CHECK(!router.HasHandler(12345));
    CHECK(router.HandlerCount() == 3);

    // Valid packets
    ENetPacket* p1 =
        Engine::Multiplayer::NetworkHostResource::CreatePacket(keyId, KeyPress{'x'}, true);
    router.Dispatch(registry, nullptr, p1->data, p1->dataLength);
    enet_packet_destroy(p1);
    CHECK(keyCalls == 1);
    CHECK(receivedKey == 'x');

    MoveTo sent{7, Engine::Position(1.5f, -2.5f)};
    ENetPacket* p2 = Engine::Multiplayer::NetworkHostResource::CreatePacket(moveId, sent, false);
    router.Dispatch(registry, nullptr, p2->data, p2->dataLength);
    enet_packet_destroy(p2);
    CHECK(moveCalls == 1);
    CHECK(receivedMove.id == 7);
    CHECK(receivedMove.pos.x == 1.5f);
    CHECK(receivedMove.pos.y == -2.5f);

    // Raw packet round-trip (header + bytes, as Net.send builds them).
    const uint8_t rawPayload[3] = {'a', 'b', 'c'};
    ENetPacket* pRaw = Engine::Multiplayer::NetworkHostResource::CreateRawPacket(
        rawId, rawPayload, sizeof(rawPayload), false);
    router.Dispatch(registry, nullptr, pRaw->data, pRaw->dataLength);
    enet_packet_destroy(pRaw);
    CHECK(rawCalls == 1);

    // Unregister removes the handler.
    CHECK(router.Off(rawId));
    CHECK(!router.HasHandler(rawId));
    CHECK(!router.Off(rawId));

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
