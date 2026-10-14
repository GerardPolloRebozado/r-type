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
    auto& networkHost = registry.GetResource<Multiplayer::NetworkHostResource>();
    auto& router = registry.GetResource<Multiplayer::PacketRouterResource>();
    networkHost.Poll(0, router, registry);
}
} // namespace Engine
