/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** ENetContext.hpp
*/
#ifndef G_ING_500_BAR_5_1_RTYPE_1_MULTIPLAYERPLUGIN_HPP
#define G_ING_500_BAR_5_1_RTYPE_1_MULTIPLAYERPLUGIN_HPP
#include "enet.h"

#include <stdexcept>

/**
 * @file ENetContext.hpp
 * @brief RAII guard for the ENet multiplayer library.
 */
namespace Engine::Multiplayer {

/**
 * @class ENetContext
 * @brief Initialises and shuts down ENet automatically.
 *
 * Lifetime determines ENet availability: construct once before creating hosts or
 * clients, and keep it alive until all networking resources have been destroyed.
 */
struct ENetContext {
    /**
     * @brief Initialises the ENet library.
     * @throws std::runtime_error when @ref enet_initialize fails.
     */
    ENetContext() {
        if (enet_initialize() != 0) {
            throw std::runtime_error("Failed to initialize ENet.");
        }
    }

    /**
     * @brief Deinitialises ENet.
     */
    ~ENetContext() {
        enet_deinitialize();
    }

    ENetContext(const ENetContext&) = delete;

    ENetContext& operator=(const ENetContext&) = delete;
};

} // namespace Engine::Multiplayer
#endif // G_ING_500_BAR_5_1_RTYPE_1_MULTIPLAYERPLUGIN_HPP