#include "Scripting/ScriptPlugin.hpp"
#include "ECS/Registry.hpp"
#include "Scripting/ScriptComponent.hpp"
#include "Scripting/ScriptSystem.hpp"

namespace Engine {

void ScriptPlugin::Init(Registry& registry) {
    registry.RegisterComponent<ScriptComponent>();
    registry.RegisterSystem<ScriptSystem>();
}

void ScriptPlugin::AttachToEntity(Registry& registry, Entity entity,
                                  const ScriptComponent& script) {
    registry.AddComponent(entity, script);
}

} // namespace Engine