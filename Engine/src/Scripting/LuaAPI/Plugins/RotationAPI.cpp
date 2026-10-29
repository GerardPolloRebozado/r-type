#include "Scripting/LuaAPI/Plugins/RotationAPI.hpp"
#include "Components/Rotation.hpp"
#include "ECS/Entity.hpp"
#include "ECS/Registry.hpp"
#include "Scripting/LuaAPI/Internal/Helpers.hpp"
#include <sol/sol.hpp>

namespace Engine::LuaAPI {

void registerRotation(sol::state_view lua, Registry& registry) {
    sol::table transform = Internal::ensureNamespace(lua, "Transform");

    transform["getRotation"] = [&registry](Entity entity) -> float {
        if (registry.HasComponent<Rotation>(entity)) {
            return registry.RequireComponent<Rotation>(entity).angle;
        }
        return 0.0f;
    };

    transform["setRotation"] = [&registry](Entity entity, float angle) {
        if (!registry.HasComponent<Rotation>(entity)) {
            registry.AddComponent<Rotation>(entity, Rotation{angle});
        } else {
            registry.RequireComponent<Rotation>(entity).angle = angle;
        }
    };

    transform["rotate"] = [&registry](Entity entity, float deltaAngle) {
        if (!registry.HasComponent<Rotation>(entity)) {
            registry.AddComponent<Rotation>(entity, Rotation{deltaAngle});
        } else {
            registry.RequireComponent<Rotation>(entity).angle += deltaAngle;
        }
    };
}

} // namespace Engine::LuaAPI
