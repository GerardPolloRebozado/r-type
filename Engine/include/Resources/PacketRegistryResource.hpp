/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** PacketRegistryResource.hpp
*/
#ifndef G_ING_500_BAR_5_1_RTYPE_1_PACKETREGISTRYRESOURCE_HPP
#define G_ING_500_BAR_5_1_RTYPE_1_PACKETREGISTRYRESOURCE_HPP
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
using PacketTypeID = std::uint16_t;

namespace Engine::Multiplayer {
constexpr PacketTypeID Ping = 0;
constexpr PacketTypeID Connect = 2;
constexpr PacketTypeID Disconnect = 3;
constexpr PacketTypeID UserDefinedStart = 200; // Boundary: IDs below are reserved
constexpr PacketTypeID MaxPacketId = 0xFFFF;

/**
 * @brief Deterministic hash using the packet name
 * std lib hash is implementation defined so a windows binary would not match a linux one
 **/
inline PacketTypeID HashPacketName(const std::string_view name) {
    std::uint32_t hash = 2166136261u;
    for (const char c : name) {
        hash ^= static_cast<std::uint32_t>(static_cast<unsigned char>(c));
        hash *= 16777619u;
    }
    constexpr std::uint32_t kRange =
        static_cast<std::uint32_t>(MaxPacketId) - static_cast<std::uint32_t>(UserDefinedStart);
    return static_cast<PacketTypeID>(UserDefinedStart + hash % (kRange + 1u));
}

/**
 * @brief stores the hash of a packet name
 */
class PacketRegistryResource {
    std::unordered_map<std::string, PacketTypeID> m_nameToId{};
    std::unordered_map<PacketTypeID, std::string> m_idToName{};

  public:
    PacketRegistryResource() = default;

    ~PacketRegistryResource() = default;

    // Delete copy and move operations to enforce single instance and avoid confusion with copy and
    // move operators
    PacketRegistryResource(const PacketRegistryResource&) = delete;

    PacketRegistryResource(PacketRegistryResource&&) = delete;

    PacketRegistryResource& operator=(const PacketRegistryResource&) = delete;

    PacketRegistryResource& operator=(PacketRegistryResource&&) = delete;

    /**
     * @brief Returns the deterministic ID for a name, registering it on first use
     */
    PacketTypeID GetOrRegister(const std::string& name) {
        if (const auto it = m_nameToId.find(name); it != m_nameToId.end()) {
            return it->second;
        }
        const PacketTypeID id = HashPacketName(name);
        m_nameToId[name] = id;
        m_idToName[id] = name;
        return id;
    }

    std::optional<PacketTypeID> TryGet(const std::string& name) const {
        if (const auto it = m_nameToId.find(name); it != m_nameToId.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    std::optional<std::string> GetName(PacketTypeID id) const {
        if (const auto it = m_idToName.find(id); it != m_idToName.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    bool Contains(const std::string& name) const {
        return m_nameToId.contains(name);
    }

    std::size_t Size() const {
        return m_nameToId.size();
    }
};
} // namespace Engine::Multiplayer
#endif // G_ING_500_BAR_5_1_RTYPE_1_PACKETREGISTRYRESOURCE_HPP
