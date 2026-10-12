/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** PacketRouter.hpp
*/
#ifndef G_ING_500_BAR_5_1_RTYPE_1_PACKETROUTER_HPP
#define G_ING_500_BAR_5_1_RTYPE_1_PACKETROUTER_HPP

#include <bitsery/adapter/buffer.h>
#include <bitsery/bitsery.h>
#include <functional>
#include <unordered_map>
#include <vector>
// DO NOT DELETE THIS INCLUDE OR IT WILL NOT COMPILE
#include "enet.h"
#include <bitsery/traits/vector.h>

using PacketTypeID = uint16_t;

namespace Engine::Multiplayer::Packets {
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

/**
 * Types used for the serializer/deserializer
 */
using InputAdapter = bitsery::InputBufferAdapter<const uint8_t*>;
using Deserializer = bitsery::Deserializer<InputAdapter>;
using Buffer = std::vector<uint8_t>;
using OutputAdapter = bitsery::OutputBufferAdapter<Buffer>;

class PacketRouterResource {
  public:
    using RawHandler = std::function<void(Registry&, ENetPeer*, Deserializer&)>;
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
                                                          Deserializer& des) {
            TData data{};
            des.object(data);
            if (des.adapter().error() == bitsery::ReaderError::NoError) {
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
        if (size < sizeof(PacketTypeID)) {
            return;
        }

        Deserializer des{InputAdapter{data, data + size}};

        PacketTypeID id = 0;
        des.value2b(id);

        if (des.adapter().error() != bitsery::ReaderError::NoError) {
            return;
        }

        auto it = m_handlers.find(id);
        if (it != m_handlers.end()) {
            it->second(registry, peer, des);
        }
    }

    PacketRouterResource(const std::function<void(Registry&, ENetPeer*)>& onConnect,
                         const std::function<void(Registry&, ENetPeer*)>& onDisconnect)
        : m_OnConnect(onConnect), m_OnDisconnect(onDisconnect) {}

  private:
    std::unordered_map<PacketTypeID, RawHandler> m_handlers;
};

} // namespace Engine::Multiplayer::Packets

#endif // G_ING_500_BAR_5_1_RTYPE_1_PACKETROUTER_HPP