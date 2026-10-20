#include "Plugins/ReplicationPlugin.hpp"

#include "Components/NetworkComponent.hpp"
#include "ECS/Registry.hpp"
#include "Resources/NetworkHostResource.hpp"
#include "Systems/ReplicationSystem.hpp"

namespace Engine::Multiplayer {

void ReplicationPlugin::Init(Registry& registry) {
    registry.RegisterComponent<NetworkComponent>();
    if (!registry.HasResource<NetworkHostResource>()) {
        // The host is created by scripts (Net.startServer/Net.startClient)
        // register the component type so it can be added later.
    }
    if (!registry.GetSystems().contains(typeid(ReplicationSystem))) {
        registry.RegisterSystem<ReplicationSystem>();
    }
}

} // namespace Engine::Multiplayer
