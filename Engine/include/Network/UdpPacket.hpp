#ifndef ENGINE_NETWORK_UDP_PACKET_HPP_
#define ENGINE_NETWORK_UDP_PACKET_HPP_

#include "Compat/EnetWrapper.hpp"
#include <cstdint>
#include <vector>

namespace Engine::Network {

enum class NetworkEventType : uint8_t { None = 0, Connect, Receive, Disconnect };

struct NetworkEvent {
    NetworkEventType type{NetworkEventType::None};
    ENetPeer* peer{nullptr};
    uint8_t channel{0};
    std::vector<uint8_t> data;
};

} // namespace Engine::Network

#endif // ENGINE_NETWORK_UDP_PACKET_HPP_
