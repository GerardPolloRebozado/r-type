#include "Scripting/LuaAPI/Plugins/TextAPI.hpp"
#include "Components/Text.hpp"
#include "ECS/Entity.hpp"
#include "ECS/Registry.hpp"
#include "Scripting/LuaAPI/Internal/Helpers.hpp"
#include <sol/sol.hpp>
#include <string>

namespace Engine::LuaAPI {

void registerText(sol::state_view lua, Registry& registry) {
    sol::table component = Internal::ensureNamespace(lua, "Component");

    component["addText"] = [&registry](Entity entity, const std::string& text, float fontSize,
                                       int r, int g, int b, int a) {
        Internal::ensureTransform(registry, entity, false);
        Text tc;
        tc.text = text;
        tc.fontsize = fontSize;
        tc.color = raylib::Color{static_cast<unsigned char>(r), static_cast<unsigned char>(g),
                                 static_cast<unsigned char>(b), static_cast<unsigned char>(a)};
        registry.AddComponent<Text>(entity, tc);
    };

    component["setText"] = [&registry](Entity entity, const std::string& text) -> bool {
        if (!registry.HasComponent<Text>(entity)) {
            return false;
        }
        registry.RequireComponent<Text>(entity).text = text;
        return true;
    };

    component["getText"] = [lua, &registry](Entity entity) mutable -> sol::object {
        if (!registry.HasComponent<Text>(entity)) {
            return sol::make_object(lua, sol::lua_nil);
        }
        return sol::make_object(lua, registry.RequireComponent<Text>(entity).text);
    };
}

} // namespace Engine::LuaAPI
