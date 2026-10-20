#include "Scripting/LuaAPI/EntityAPI.hpp"
#include "Components/Name.hpp"
#include "ECS/Entity.hpp"
#include "ECS/Registry.hpp"
#include "Scripting/LuaAPI/Internal/Helpers.hpp"
#include <sol/sol.hpp>
#include <string>

namespace Engine::LuaAPI {

void registerEntity(sol::state_view lua, Registry& registry) {
    sol::table t = Internal::ensureNamespace(lua, "Entity");

    t["new"] = [](sol::object id) -> Entity {
        if (id.is<Entity>()) {
            return id.as<Entity>();
        }
        if (id.is<int>()) {
            return static_cast<Entity>(id.as<int>());
        }
        if (id.is<lua_Number>()) {
            return static_cast<Entity>(id.as<lua_Number>());
        }
        return static_cast<Entity>(0);
    };

    t["create"] = [&registry]() -> Entity { return registry.CreateEntity(); };

    t["destroy"] = [&registry](Entity entity) { registry.DestroyEntity(entity); };

    t["queueDestroy"] = [&registry](Entity entity) { registry.QueueDestroyEntity(entity); };

    t["find"] = [lua, &registry](const std::string& name) mutable -> sol::object {
        for (size_t i = 0; i < registry.GetEntityCount(); ++i) {
            Entity e{i};
            if (registry.HasComponent<Name>(e) && registry.GetComponent<Name>(e).value == name) {
                return sol::make_object(lua, e);
            }
        }
        return sol::make_object(lua, sol::lua_nil);
    };

    t["getName"] = [&registry](Entity entity) -> std::string {
        if (registry.HasComponent<Name>(entity)) {
            return registry.GetComponent<Name>(entity).value;
        }
        return "";
    };

    t["setName"] = [&registry](Entity entity, const std::string& name) {
        if (!registry.HasComponent<Name>(entity)) {
            registry.AddComponent<Name>(entity, Name{name});
        } else {
            registry.GetComponent<Name>(entity).value = name;
        }
    };
}

} // namespace Engine::LuaAPI
