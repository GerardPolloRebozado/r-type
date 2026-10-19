/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** MultiplayerSystem.cpp
*/

#include "Systems/MultiplayerSystem.hpp"

#include "ECS/Registry.hpp"
#include "Resources/NetworkHostResource.hpp"

namespace Engine {
void MultiplayerSystem::Update(Registry& registry, float dt) {
    // The network host is optional: a game may create it later from Lua
    // (Net.startServer / Net.startClient). Poll nothing until it exists.
    if (!registry.HasResource<Multiplayer::NetworkHostResource>()) {
        return;
    }
    auto& networkHost = registry.GetResource<Multiplayer::NetworkHostResource>();
    auto& router = registry.GetResource<Multiplayer::PacketRouterResource>();
    networkHost.Poll(0, router, registry);
}
} // namespace Engine
