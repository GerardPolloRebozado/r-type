#include "Plugins/PluginExample.hpp"

namespace Engine {
void PluginExample::Init(Registry& registry) {
    registry.RegisterComponent<ComponentExample>();
    registry.RegisterComponent<PluginExampleEvents>();
    registry.RegisterSystem<SystemExample>();
}

void PluginExample::AttachToEntity(Registry& registry, Entity entity) {
    registry.AddComponent(entity, ComponentExample{0.0f, 0});
    registry.AddComponent(entity, PluginExampleEvents{});
}
}
