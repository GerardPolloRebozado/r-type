#include "Plugins/CameraPlugin.hpp"

namespace Engine {

void CameraPlugin::Init(Registry& registry) {
    registry.RegisterComponent<Position>();
    registry.RegisterComponent<CameraComponent>();
    registry.RegisterSystem<CameraSystem>();
}

void CameraPlugin::AttachToEntity(Registry& registry, Entity entity,
                                  const CameraComponent& camera) {
    if (!registry.HasComponent<Position>(entity)) {
        registry.AddComponent<Position>(entity, Position{0, 0});
    }
    registry.AddComponent<CameraComponent>(entity, camera);
}

} // namespace Engine
