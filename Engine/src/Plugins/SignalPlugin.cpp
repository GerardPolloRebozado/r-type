#include "Plugins/SignalPlugin.hpp"

namespace Engine {

void SignalPlugin::Init(Registry& registry) {
    registry.RegisterComponent<SignalConnections>();
}

void SignalPlugin::AttachToEntity(Registry& registry, Entity entity) {
    registry.RegisterComponent<SignalConnections>();
    if (!registry.HasComponent<SignalConnections>(entity)) {
        registry.AddComponent<SignalConnections>(entity, SignalConnections{});
    }
}

} // namespace Engine
