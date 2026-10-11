/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** MultiplayerPlugin.hpp
*/
#ifndef G_ING_500_BAR_5_1_RTYPE_1_MULTIPLAYERPLUGIN_HPP
#define G_ING_500_BAR_5_1_RTYPE_1_MULTIPLAYERPLUGIN_HPP
#include "enet.h"

#include <stdexcept>

namespace Engine::Multiplayer {

struct ENetContext {
    ENetContext() {
        if (enet_initialize() != 0) {
            throw std::runtime_error("Failed to initialize ENet.");
        }
    }

    ~ENetContext() {
        enet_deinitialize();
    }

    ENetContext(const ENetContext&) = delete;

    ENetContext& operator=(const ENetContext&) = delete;
};

} // namespace Engine::Multiplayer
#endif // G_ING_500_BAR_5_1_RTYPE_1_MULTIPLAYERPLUGIN_HPP