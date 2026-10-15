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
}

} // namespace Engine::LuaAPI
