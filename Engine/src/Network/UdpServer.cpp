#include "Network/UdpServer.hpp"
#include <chrono>
#include <iostream>

namespace Engine::Network {

UdpServer::UdpServer() = default;

UdpServer::~UdpServer() {
    Stop();
}

bool UdpServer::Start(uint16_t port, size_t maxClients, size_t channels, bool multiThreaded) {
    if (isRunning_.load()) {
        return true;
    }

    if (enet_initialize() != 0) {
        std::cerr << "[Engine::Network] Failed to initialize ENet subsystem." << std::endl;
        return false;
    }

    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = port;

    serverHost_ = enet_host_create(&address, maxClients, channels, 0, 0);
    if (!serverHost_) {
        std::cerr << "[Engine::Network] Failed to bind UDP server on port " << port << std::endl;
        return false;
    }

    port_ = port;
    multiThreaded_ = multiThreaded;
    isRunning_.store(true);

    if (multiThreaded_) {
        ioThread_ = std::thread(&UdpServer::NetworkIoLoop, this);
        std::cout << "[Engine::Network] Multi-threaded UDP Server listening on port " << port
                  << std::endl;
    } else {
        std::cout << "[Engine::Network] Single-threaded UDP Server listening on port " << port
                  << std::endl;
    }

    return true;
}

void UdpServer::Stop() {
    if (!isRunning_.exchange(false)) {
        return;
    }

    if (ioThread_.joinable()) {
        ioThread_.join();
    }

    if (serverHost_) {
        std::lock_guard<std::mutex> lock(sendMutex_);
        for (size_t i = 0; i < serverHost_->peerCount; ++i) {
            ENetPeer* peer = &serverHost_->peers[i];
            if (peer->state != ENET_PEER_STATE_DISCONNECTED) {
                enet_peer_disconnect_now(peer, 0);
            }
        }
        enet_host_flush(serverHost_);
        enet_host_destroy(serverHost_);
        serverHost_ = nullptr;
    }

    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        std::queue<NetworkEvent> empty;
        std::swap(incomingEvents_, empty);
    }

    enet_deinitialize();
    std::cout << "[Engine::Network] Server stopped." << std::endl;
}

size_t UdpServer::GetConnectedCount() const {
    if (!serverHost_) {
        return 0;
    }
    std::lock_guard<std::mutex> lock(sendMutex_);
    size_t count = 0;
    for (size_t i = 0; i < serverHost_->peerCount; ++i) {
        if (serverHost_->peers[i].state == ENET_PEER_STATE_CONNECTED) {
            count++;
        }
    }
    return count;
}

void UdpServer::NetworkIoLoop() {
    while (isRunning_.load()) {
        ProcessEnetEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
}

void UdpServer::ProcessEnetEvents() {
    if (!serverHost_) {
        return;
    }

    std::lock_guard<std::mutex> lock(sendMutex_);
    ENetEvent event;
    while (enet_host_service(serverHost_, &event, 0) > 0) {
        NetworkEvent netEvt;
        netEvt.peer = event.peer;
        netEvt.channel = event.channelID;

        switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
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
                netEvt.type = NetworkEventType::Disconnect;
                break;
            case ENET_EVENT_TYPE_NONE:
                break;
        }

        if (netEvt.type != NetworkEventType::None) {
            std::lock_guard<std::mutex> qLock(queueMutex_);
            incomingEvents_.push(std::move(netEvt));
        }
    }
}

void UdpServer::PollEvents(const std::function<void(const NetworkEvent&)>& handler) {
    if (!multiThreaded_ && isRunning_.load()) {
        ProcessEnetEvents();
    }

    std::queue<NetworkEvent> localQueue;
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        std::swap(localQueue, incomingEvents_);
    }

    while (!localQueue.empty()) {
        handler(localQueue.front());
        localQueue.pop();
    }
}

void UdpServer::SendPacket(ENetPeer* peer, const uint8_t* data, size_t size, bool reliable,
                           uint8_t channel) {
    if (!peer || !serverHost_ || !data || size == 0) {
        return;
    }
    std::lock_guard<std::mutex> lock(sendMutex_);
    enet_uint32 flags = reliable ? ENET_PACKET_FLAG_RELIABLE : 0;
    ENetPacket* packet = enet_packet_create(data, size, flags);
    enet_peer_send(peer, channel, packet);
}

void UdpServer::SendPacket(ENetPeer* peer, const ByteWriter& writer, bool reliable,
                           uint8_t channel) {
    SendPacket(peer, writer.Data(), writer.Size(), reliable, channel);
}

void UdpServer::BroadcastPacket(const uint8_t* data, size_t size, bool reliable, uint8_t channel) {
    if (!serverHost_ || !data || size == 0) {
        return;
    }
    std::lock_guard<std::mutex> lock(sendMutex_);
    enet_uint32 flags = reliable ? ENET_PACKET_FLAG_RELIABLE : 0;
    ENetPacket* packet = enet_packet_create(data, size, flags);
    enet_host_broadcast(serverHost_, channel, packet);
}

void UdpServer::BroadcastPacket(const ByteWriter& writer, bool reliable, uint8_t channel) {
    BroadcastPacket(writer.Data(), writer.Size(), reliable, channel);
}

void UdpServer::DisconnectPeer(ENetPeer* peer, uint32_t data) {
    if (!peer || !serverHost_) {
        return;
    }
    std::lock_guard<std::mutex> lock(sendMutex_);
    enet_peer_disconnect(peer, data);
}

} // namespace Engine::Network
