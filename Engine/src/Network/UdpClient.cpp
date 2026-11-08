#include "Network/UdpClient.hpp"
#include <iostream>

namespace Engine::Network {

UdpClient::UdpClient() = default;

UdpClient::~UdpClient() {
    Disconnect();
}

bool UdpClient::Connect(const std::string& hostAddress, uint16_t port, size_t channels) {
    if (clientHost_) {
        Disconnect();
    }

    if (enet_initialize() != 0) {
        std::cerr << "[Engine::Network] Failed to initialize ENet client subsystem." << std::endl;
        return false;
    }

    clientHost_ = enet_host_create(nullptr, 1, channels, 0, 0);
    if (!clientHost_) {
        std::cerr << "[Engine::Network] Failed to create ENet client host." << std::endl;
        return false;
    }

    ENetAddress address;
    enet_address_set_host(&address, hostAddress.c_str());
    address.port = port;

    serverPeer_ = enet_host_connect(clientHost_, &address, channels, 0);
    if (!serverPeer_) {
        std::cerr << "[Engine::Network] Failed to initiate connection to " << hostAddress << ":"
                  << port << std::endl;
        enet_host_destroy(clientHost_);
        clientHost_ = nullptr;
        return false;
    }

    state_ = ConnectionState::Connecting;
    return true;
}

void UdpClient::Disconnect() {
    if (serverPeer_) {
        enet_peer_disconnect_now(serverPeer_, 0);
        serverPeer_ = nullptr;
    }

    if (clientHost_) {
        enet_host_destroy(clientHost_);
        clientHost_ = nullptr;
        enet_deinitialize();
    }

    state_ = ConnectionState::Disconnected;
}

uint32_t UdpClient::GetPing() const noexcept {
    if (serverPeer_) {
        return serverPeer_->roundTripTime;
    }
    return 0;
}

void UdpClient::PollEvents(const std::function<void(const NetworkEvent&)>& handler) {
    if (!clientHost_) {
        return;
    }

    ENetEvent event;
    while (enet_host_service(clientHost_, &event, 0) > 0) {
        NetworkEvent netEvt;
        netEvt.peer = event.peer;
        netEvt.channel = event.channelID;

        switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                state_ = ConnectionState::Connected;
                netEvt.type = NetworkEventType::Connect;
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                netEvt.type = NetworkEventType::Receive;
                if (event.packet && event.packet->data && event.packet->dataLength > 0) {
                    netEvt.data.assign(event.packet->data,
                                       event.packet->data + event.packet->dataLength);
                }
                enet_packet_destroy(event.packet);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                state_ = ConnectionState::Disconnected;
                netEvt.type = NetworkEventType::Disconnect;
                break;
            case ENET_EVENT_TYPE_NONE:
                break;
        }

        if (netEvt.type != NetworkEventType::None) {
            handler(netEvt);
        }
    }
}

void UdpClient::SendPacket(const uint8_t* data, size_t size, bool reliable, uint8_t channel) {
    if (!serverPeer_ || !clientHost_ || !data || size == 0) {
        return;
    }
    enet_uint32 flags = reliable ? ENET_PACKET_FLAG_RELIABLE : 0;
    ENetPacket* packet = enet_packet_create(data, size, flags);
    enet_peer_send(serverPeer_, channel, packet);
}

void UdpClient::SendPacket(const ByteWriter& writer, bool reliable, uint8_t channel) {
    SendPacket(writer.Data(), writer.Size(), reliable, channel);
}

} // namespace Engine::Network
