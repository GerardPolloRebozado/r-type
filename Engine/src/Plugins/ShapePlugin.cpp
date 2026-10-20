#include "Plugins/ShapePlugin.hpp"

#include "Plugins/OpacityPlugin.hpp"

namespace Engine {

void ShapePlugin::Init(Registry& registry) {
    registry.RegisterComponent<Position>();
    registry.RegisterComponent<ShapeComponent>();
    registry.RegisterSystem<ShapeSystem>();
    OpacityPlugin opacityPlugin;
    opacityPlugin.Init(registry);
}

void ShapePlugin::AttachToEntity(Registry& registry, Entity entity, const ShapeComponent& shape) {
    if (!registry.HasComponent<Position>(entity)) {
        registry.AddComponent<Position>(entity, Position(0, 0));
    }
    registry.AddComponent<ShapeComponent>(entity, shape);
}

} // namespace Engine