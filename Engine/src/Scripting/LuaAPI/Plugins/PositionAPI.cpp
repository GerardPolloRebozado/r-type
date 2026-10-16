#include "Scripting/LuaAPI/Plugins/PositionAPI.hpp"
#include "Components/Position.hpp"
#include "ECS/Entity.hpp"
#include "ECS/Registry.hpp"
#include "Scripting/LuaAPI/Internal/Helpers.hpp"
#include <sol/sol.hpp>

namespace Engine::LuaAPI {

void registerPosition(sol::state_view lua, Registry& registry) {
    sol::table transform = Internal::ensureNamespace(lua, "Transform");

    // Returns a plain {x, y} table rather than the C++ Position usertype to
    // avoid sol2 usertype copy-construction friction with Position's extra
    // fields (prevX/prevY/moved/relative), or whatever I read on a so post
    transform["getPosition"] = [lua, &registry](Entity entity) mutable -> sol::table {
        sol::table t = lua.create_table();
        t["x"] = 0.0f;
        t["y"] = 0.0f;
        if (registry.HasComponent<Position>(entity)) {
            auto& p = registry.GetComponent<Position>(entity);
            t["x"] = p.x;
            t["y"] = p.y;
        }
        return t;
    };

    transform["setPosition"] = [&registry](Entity entity, float x, float y) {
        if (!registry.HasComponent<Position>(entity)) {
            registry.AddComponent<Position>(entity, Position{x, y});
        } else {
            auto& pos = registry.GetComponent<Position>(entity);
            pos.x = x;
            pos.y = y;
        }
    };

    transform["translate"] = [&registry](Entity entity, float dx, float dy) {
        if (!registry.HasComponent<Position>(entity)) {
            registry.AddComponent<Position>(entity, Position{dx, dy});
        } else {
            auto& pos = registry.GetComponent<Position>(entity);
            pos.x += dx;
            pos.y += dy;
        }
    };
}

} // namespace Engine::LuaAPI
