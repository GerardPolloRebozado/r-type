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
#include <typeindex>
#include <unordered_map>

namespace Engine::LuaAPI {

void registerComponentQuery(sol::state_view lua, Registry& registry) {
    sol::table component = Internal::ensureNamespace(lua, "Component");

    // Lua uses friendly alias names, while the ECS registry is keyed by
    // std::type_index (typeid(T)). This table is the bridge between the two so we
    // don't need a hand-written if-chain for every component visible in scripts.
    static const std::unordered_map<std::string, std::type_index> kComponentTypeIds = {
        {"Position", std::type_index(typeid(Position))},
        {"Rotation", std::type_index(typeid(Rotation))},
        {"Sprite", std::type_index(typeid(SpriteComponent))},
        {"Shape", std::type_index(typeid(ShapeComponent))},
        {"Text", std::type_index(typeid(Text))},
        {"Name", std::type_index(typeid(Name))},
        {"Collider", std::type_index(typeid(ColliderComponent))},
        {"Parent", std::type_index(typeid(Parent))},
    };

    component["has"] = [&registry](Entity entity, const std::string& typeName) -> bool {
        const auto it = kComponentTypeIds.find(typeName);
        if (it == kComponentTypeIds.end()) {
            return false;
        }
        return registry.HasComponentByType(it->second, entity);
    };

    component["remove"] = [&registry](Entity entity, const std::string& typeName) {
        const auto it = kComponentTypeIds.find(typeName);
        if (it == kComponentTypeIds.end()) {
            return;
        }
        registry.RemoveComponentByType(it->second, entity);
    };
}

} // namespace Engine::LuaAPI
