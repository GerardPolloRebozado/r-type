#include "Scripting/LuaAPI/Plugins/PositionAPI.hpp"
#include "Components/Position.hpp"
#include "ECS/Entity.hpp"
#include "ECS/Registry.hpp"
#include "Scripting/LuaAPI/Internal/Helpers.hpp"
#include <sol/sol.hpp>

namespace Engine::LuaAPI {

void registerPosition(sol::state_view lua, Registry& registry) {
    sol::table transform = Internal::ensureNamespace(lua, "Transform");

    // Returns a plain {x, y, z} table rather than the C++ Position usertype to
    // avoid sol2 usertype copy-construction friction with Position's extra
    // fields (prevX/prevY/moved/relative), or whatever I read on a so post
    transform["getPosition"] = [lua, &registry](Entity entity) mutable -> sol::table {
        sol::table t = lua.create_table();
        t["x"] = 0.0f;
        t["y"] = 0.0f;
        t["z"] = 0;
        if (registry.HasComponent<Position>(entity)) {
            auto& p = registry.GetComponent<Position>(entity);
            t["x"] = p.x;
            t["y"] = p.y;
            t["z"] = p.z;
        }
        return t;
    };

    // z is optional: when omitted, the entity keeps its current draw layer.
    transform["setPosition"] = [&registry](Entity entity, float x, float y,
                                           sol::optional<int> z) {
        if (!registry.HasComponent<Position>(entity)) {
            registry.AddComponent<Position>(entity, Position{x, y, z.value_or(0)});
        } else {
            auto& pos = registry.GetComponent<Position>(entity);
            pos.x = x;
            pos.y = y;
            if (z) {
                pos.z = *z;
            }
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

    // Draw layer: higher z is drawn on top. 0 when the entity has no Position.
    transform["getZIndex"] = [&registry](Entity entity) -> int {
        if (registry.HasComponent<Position>(entity)) {
            return registry.GetComponent<Position>(entity).z;
        }
        return 0;
    };

    transform["setZIndex"] = [&registry](Entity entity, int z) {
        if (!registry.HasComponent<Position>(entity)) {
            registry.AddComponent<Position>(entity, Position{0.0f, 0.0f, z});
        } else {
            registry.GetComponent<Position>(entity).z = z;
        }
    };
}

} // namespace Engine::LuaAPI
