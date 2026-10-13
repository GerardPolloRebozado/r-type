#include "Scripting/LuaAPI/ComponentQueryAPI.hpp"
#include "Components/ColliderComponent.hpp"
#include "Components/Name.hpp"
#include "Components/Parent.hpp"
#include "Components/Position.hpp"
#include "Components/Rotation.hpp"
#include "Components/ShapeComponent.hpp"
#include "Components/SpriteComponent.hpp"
#include "Components/Text.hpp"
#include "ECS/Entity.hpp"
#include "ECS/Registry.hpp"
#include "Scripting/LuaAPI/Internal/Helpers.hpp"
#include <sol/sol.hpp>
#include <string>

namespace Engine::LuaAPI {

void registerComponentQuery(sol::state_view lua, Registry& registry) {
    sol::table component = Internal::ensureNamespace(lua, "Component");

    // yes, the if statements are the best way of doing it. no a map with lambdas doesn't work, and
    // looks ugly and harder to read no, you can't have a switch statement with a string. you could
    // be doing oneline ifs but the formatter will change it.
    component["has"] = [&registry](Entity entity, const std::string& typeName) -> bool {
        if (typeName == "Position") {
            return registry.HasComponent<Position>(entity);
        }
        if (typeName == "Rotation") {
            return registry.HasComponent<Rotation>(entity);
        }
        if (typeName == "Sprite") {
            return registry.HasComponent<SpriteComponent>(entity);
        }
        if (typeName == "Shape") {
            return registry.HasComponent<ShapeComponent>(entity);
        }
        if (typeName == "Text") {
            return registry.HasComponent<Text>(entity);
        }
        if (typeName == "Name") {
            return registry.HasComponent<Name>(entity);
        }
        if (typeName == "Collider") {
            return registry.HasComponent<ColliderComponent>(entity);
        }
        if (typeName == "Parent") {
            return registry.HasComponent<Parent>(entity);
        }
        return false;
    };

    component["remove"] = [&registry](Entity entity, const std::string& typeName) {
        if (typeName == "Position") {
            registry.RemoveComponent<Position>(entity);
        } else if (typeName == "Rotation") {
            registry.RemoveComponent<Rotation>(entity);
        } else if (typeName == "Sprite") {
            registry.RemoveComponent<SpriteComponent>(entity);
        } else if (typeName == "Shape") {
            registry.RemoveComponent<ShapeComponent>(entity);
        } else if (typeName == "Text") {
            registry.RemoveComponent<Text>(entity);
        } else if (typeName == "Name") {
            registry.RemoveComponent<Name>(entity);
        } else if (typeName == "Collider") {
            registry.RemoveComponent<ColliderComponent>(entity);
        } else if (typeName == "Parent") {
            registry.RemoveComponent<Parent>(entity);
        }
    };
}

} // namespace Engine::LuaAPI
