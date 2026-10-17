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
#include "Compat/EnetWrapper.hpp"
#include "PacketRegistryResource.hpp"
#include "Reflection/ComponentReflect.hpp"
#include <glaze/glaze.hpp>

namespace Engine::Multiplayer {
struct PlayerInformation {
    std::size_t id = 0;
};

struct YourPlayerId {
    std::size_t id;
};

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
     * @brief Subscribes a handler to a packet id: handler runs every time a
     * packet of this type arrives
     * @param id packet id (use PacketRegistryResource::GetOrRegister(name))
     * @param handler callback receiving (registry, peer, payload, size)
     */
    void On(const PacketTypeID id, RawHandler handler) {
        m_handlers[id] = std::move(handler);
    }

    /**
     * @brief Removes the handler for a packet id. Returns true if one existed.
     */
    bool Off(const PacketTypeID id) {
        return m_handlers.erase(id) > 0;
    }

    bool HasHandler(const PacketTypeID id) const {
        return m_handlers.contains(id);
    }

    std::size_t HandlerCount() const {
        return m_handlers.size();
    }

    /**
     * @brief given a packet it will get its id and find it's handler
     * @param registry Registry of the game
     * @param peer the client that send the request
     * @param data data that the client send
     * @param size size of the data received
     */
    void Dispatch(Registry& registry, ENetPeer* peer, const uint8_t* data,
                  const size_t size) const {
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

    PacketRouterResource(

        const std::function<void(Registry&, ENetPeer*)>& onConnect,

        const std::function<void(Registry&, ENetPeer*)>& onDisconnect)
        : m_OnConnect(onConnect), m_OnDisconnect(onDisconnect) {}

    PacketRouterResource() = default;

  private:
    std::unordered_map<PacketTypeID, RawHandler> m_handlers;
};

} // namespace Engine::Multiplayer

#endif // G_ING_500_BAR_5_1_RTYPE_1_PACKETROUTER_HPP
