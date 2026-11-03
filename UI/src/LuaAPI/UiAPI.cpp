#include "UI/LuaAPI/UiAPI.hpp"
#include "UI/UiRuntime.hpp"
#include "sol_ImGui.h"

namespace Engine::LuaAPI {

void registerUI(sol::state_view lua, Registry& registry) {
    sol::table ui = lua.create_named_table("UI");

    sol_ImGui::Init(lua);

    ui["use"] = [&registry, lua](const std::string& name) {
        return registry.GetResource<UiRuntime>().UseTemplate(lua, registry, name);
    };
    ui["register"] = [&registry](const std::string& name, sol::table table) {
        registry.GetResource<UiRuntime>().RegisterTemplate(name, std::move(table));
    };
    ui["merge"] = [lua](sol::table defaults, sol::optional<sol::table> props) mutable {
        sol::table merged = lua.create_table();
        defaults.for_each([&](sol::object key, sol::object value) { merged[key] = value; });
        if (props) {
            props->for_each([&](sol::object key, sol::object value) { merged[key] = value; });
        }
        return merged;
    };
    ui["setTheme"] = [&registry](sol::table props) {
        registry.GetResource<UiRuntime>().SetTheme(props);
    };
    ui["pushStyle"] = [&registry](sol::table props) {
        registry.GetResource<UiRuntime>().PushStyle(props);
    };
    ui["popStyle"] = [&registry]() { registry.GetResource<UiRuntime>().PopStyle(); };
    ui["blocksMouse"] = [&registry]() {
        if (!registry.HasResource<UiRuntime>()) {
            return false;
        }
        return registry.GetResource<UiRuntime>().BlocksMouse();
    };
    ui["blocksKeyboard"] = [&registry]() {
        if (!registry.HasResource<UiRuntime>()) {
            return false;
        }
        return registry.GetResource<UiRuntime>().BlocksKeyboard();
    };
    ui["image"] = [&registry](const std::string& path, float width, float height) {
        registry.GetResource<UiRuntime>().DrawImage(registry, path, width, height);
    };
}

} // namespace Engine::LuaAPI
