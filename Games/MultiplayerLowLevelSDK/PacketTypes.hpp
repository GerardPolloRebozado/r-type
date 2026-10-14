/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** PacketTypes.hpp
*/
#ifndef G_ING_500_BAR_5_1_RTYPE_1_PACKETTYPES_HPP
#define G_ING_500_BAR_5_1_RTYPE_1_PACKETTYPES_HPP

#include "Components/Position.hpp"
#include "Resources/PacketRouterResource.hpp"
#include <cstddef>

struct KeyPress {
    char key = 0;
};

const PacketTypeID KeyPressPacket = Engine::Multiplayer::GetPacketTypeID<KeyPress>();

struct NewPlayerPos {
    std::size_t id{};
    Engine::Position pos;
};

const PacketTypeID NewPlayerPosPacket = Engine::Multiplayer::GetPacketTypeID<NewPlayerPos>();

struct PlayerId {
    std::size_t id;
};

const PacketTypeID PlayerIdPacket = Engine::Multiplayer::GetPacketTypeID<PlayerId>();
#endif // G_ING_500_BAR_5_1_RTYPE_1_PACKETTYPES_HPP
