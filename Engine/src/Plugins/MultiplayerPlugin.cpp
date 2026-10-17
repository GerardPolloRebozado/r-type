/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** MultiplayerPlugin.cpp
*/

#include "Plugins/MultiplayerPlugin.hpp"

#include "Resources/PacketRegistryResource.hpp"
#include "Resources/PacketRouterResource.hpp"
#include "Systems/MultiplayerSystem.hpp"

namespace Engine::Multiplayer {
void MultiplayerPlugin::Init(Registry& registry) {
    registry.RegisterComponent<PlayerInformation>();
    if (!registry.HasResource<PacketRegistryResource>()) {
        registry.AddResource<PacketRegistryResource>();
    }
    if (!registry.HasResource<PacketRouterResource>()) {
        registry.AddResource<PacketRouterResource>();
    }
    if (!registry.GetSystems().contains(typeid(MultiplayerSystem))) {
        registry.RegisterSystem<MultiplayerSystem>();
    }
}
} // namespace Engine::Multiplayer
