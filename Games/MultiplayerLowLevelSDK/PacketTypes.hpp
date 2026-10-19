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

struct NewPlayerPos {
    std::size_t id{};
    Engine::Position pos;
};

struct PlayerId {
    std::size_t id;
};

#endif // G_ING_500_BAR_5_1_RTYPE_1_PACKETTYPES_HPP