#include "Plugins/InputPlugin.hpp"
#include "Components/Input.hpp"
#include "Inputs/InputManager.hpp"

namespace Engine {
void InputPlugin::Init(Registry& registry) {
    registry.RegisterComponent<Input>();
    if (!registry.HasResource<InputManager>()) {
        registry.AddResource<InputManager>();
    }
}

void InputPlugin::AttachToEntity(Registry& registry, const Entity entity, int idxPlayer) {
    registry.AddComponent(entity, Input{idxPlayer});
}
} // namespace Engine
