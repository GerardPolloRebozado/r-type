/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** PacketRouter.hpp
*/
#ifndef G_ING_500_BAR_5_1_RTYPE_1_PACKETROUTER_HPP
#define G_ING_500_BAR_5_1_RTYPE_1_PACKETROUTER_HPP

#include <cstddef>
#include <cstdint>
#include <functional>
#include <span>
#include <unordered_map>
#include <vector>
// DO NOT DELETE THIS INCLUDE OR IT WILL NOT COMPILE
#include "enet.h"
#include <glaze/glaze.hpp>
// Glaze reflection for Engine components, so packets embedding them
// (e.g. Engine::Position) serialize with zero manual code.
#include "Reflection/ComponentReflect.hpp"

using PacketTypeID = uint16_t;

namespace Engine::Multiplayer {
constexpr PacketTypeID Ping = 0;
constexpr PacketTypeID Connect = 2;
constexpr PacketTypeID Disconnect = 3;
constexpr PacketTypeID UserDefinedStart = 200; // Boundary

inline PacketTypeID GetNextPacketID() {
    static PacketTypeID counter = UserDefinedStart;
    return ++counter;
}

template <typename T> PacketTypeID GetPacketTypeID() {
    static const PacketTypeID id = GetNextPacketID();
    return id;
}

struct PlayerInformation {
    std::size_t id;
    ENetPeer* peer;
};

const PacketTypeID PlayerInformationPacket =
    Engine::Multiplayer::GetPacketTypeID<PlayerInformation>();

struct YourPlayerId {
    std::size_t id;
};

const PacketTypeID YourPlayerIdPacket = Engine::Multiplayer::GetPacketTypeID<YourPlayerId>();

/**
 * Types used for the serializer/deserializer (glaze binary payloads).
 */
using Buffer = std::vector<std::byte>;

class PacketRouterResource {
  public:
    using RawHandler = std::function<void(Registry&, ENetPeer*, const uint8_t*, std::size_t)>;
    std::function<void(Registry&, ENetPeer*)> m_OnConnect;
    std::function<void(Registry&, ENetPeer*)> m_OnDisconnect;

    /**
     * @brief This function given a struct type and it's id it will execute the callback when the
     * type of packet arrives
     * @tparam TData Type of data to process
     * @param id id of the packet being send
     * @param callback The function to call when receiving the packet
     */
    template <typename TData>
    void RegisterHandler(const PacketTypeID id,
                         std::function<void(Registry&, ENetPeer*, const TData&)> callback) {
        m_handlers[id] = [callback = std::move(callback)](Registry& registry, ENetPeer* peer,
                                                          const uint8_t* payload,
                                                          const std::size_t size) {
            TData data{};
            const std::span<const std::byte> buffer{reinterpret_cast<const std::byte*>(payload),
                                                    size};
            if (!glz::read_beve(data, buffer)) {
                callback(registry, peer, data);
            }
        };
    }

    /**
     * @brief This function given a struct type and it's id it will execute the callback when the
     * type of packet arrives
     * @tparam TData Type of data to process
     * @param id id of the packet being send
     * @param callback The function to call when receiving the packet
     */
    template <typename TData>
    void RegisterHandler(const PacketTypeID id,
                         void (*callback)(Registry&, ENetPeer*, const TData&)) {
        RegisterHandler<TData>(id,
                               std::function<void(Registry&, ENetPeer*, const TData&)>(callback));
    }

    /**
     * @brief given a packet it will get its id and find it's handler
     * @param registry Registry of the game
     * @param peer the client that send the request
     * @param data data that the client send
     * @param size size of the data received
     */
    void Dispatch(Registry& registry, ENetPeer* peer, const uint8_t* data, const size_t size) {
        static constexpr std::size_t kHeaderSize = sizeof(PacketTypeID);
        if (size < kHeaderSize) {
            return;
        }

        const auto id = static_cast<PacketTypeID>(data[0] | (data[1] << 8));

        auto it = m_handlers.find(id);
        if (it != m_handlers.end()) {
            it->second(registry, peer, data + kHeaderSize, size - kHeaderSize);
        }
    }

    PacketRouterResource(const std::function<void(Registry&, ENetPeer*)>& onConnect,
                         const std::function<void(Registry&, ENetPeer*)>& onDisconnect)
        : m_OnConnect(onConnect), m_OnDisconnect(onDisconnect) {}

  private:
    std::unordered_map<PacketTypeID, RawHandler> m_handlers;
};
} // namespace Engine::Multiplayer

#endif // G_ING_500_BAR_5_1_RTYPE_1_PACKETROUTER_HPP
