#include "Plugins/RotationPlugin.hpp"

namespace Engine {

void RotationPlugin::Init(Registry& registry) {
    registry.RegisterComponent<Rotation>();
}

void RotationPlugin::AttachToEntity(Registry& registry, Entity entity, float angle) {
    registry.AddComponent<Rotation>(entity, Rotation::Create(angle));
}

} // namespace Engine
