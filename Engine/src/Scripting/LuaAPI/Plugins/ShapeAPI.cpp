#include "Scripting/LuaAPI/Plugins/ShapeAPI.hpp"
#include "Components/ShapeComponent.hpp"
#include "ECS/Entity.hpp"
#include "ECS/Registry.hpp"
#include "Scripting/LuaAPI/Internal/Helpers.hpp"
#include <sol/sol.hpp>

namespace Engine::LuaAPI {

void registerShape(sol::state_view lua, Registry& registry) {
    sol::table component = Internal::ensureNamespace(lua, "Component");

    component["addShape"] = [&registry](Entity entity, int shapeType, float width, float height,
                                        float radius, int sides, float thickness, bool filled,
                                        int r, int g, int b, int a) {
        Internal::ensureTransform(registry, entity, true);
        ShapeComponent shape;
        shape.type = static_cast<ShapeType>(shapeType);
        shape.width = width;
        shape.height = height;
        shape.radius = radius;
        shape.sides = sides;
        shape.thickness = thickness;
        shape.filled = filled;
        shape.color = raylib::Color{static_cast<unsigned char>(r), static_cast<unsigned char>(g),
                                    static_cast<unsigned char>(b), static_cast<unsigned char>(a)};
        registry.AddComponent<ShapeComponent>(entity, shape);
    };
}

} // namespace Engine::LuaAPI
