#include "Scripting/LuaAPI/CameraAPI.hpp"
#include "Core/VirtualResolution.hpp"
#include "ECS/Registry.hpp"
#include "Scripting/LuaAPI/Internal/Helpers.hpp"
#include "Systems/CameraSystem.hpp"
#include <raylib.h>
#include <sol/sol.hpp>

namespace Engine::LuaAPI {

void registerCamera(sol::state_view lua, Registry& registry) {
    sol::table camera = Internal::ensureNamespace(lua, "Camera");

    camera["viewport"] = [&registry, luaState = lua.lua_state()]() mutable -> sol::object {
        sol::state_view lua(luaState);
        float width = static_cast<float>(GetScreenWidth());
        float height = static_cast<float>(GetScreenHeight());
        if (registry.HasResource<VirtualResolution>()) {
            const auto& resolution = registry.GetResource<VirtualResolution>();
            width = static_cast<float>(resolution.GetWidth());
            height = static_cast<float>(resolution.GetHeight());
        }

        const Camera2D active = CameraSystem::GetActiveCamera(registry, width, height);
        const float zoom = active.zoom > 0.0f ? active.zoom : 1.0f;
        const float worldWidth = width / zoom;
        const float worldHeight = height / zoom;
        const float left = active.target.x - worldWidth * 0.5f;
        const float top = active.target.y - worldHeight * 0.5f;

        sol::table result = lua.create_table();
        result["x"] = left;
        result["y"] = top;
        result["width"] = worldWidth;
        result["height"] = worldHeight;
        result["centerX"] = active.target.x;
        result["centerY"] = active.target.y;
        result["zoom"] = zoom;
        return result;
    };
}

} // namespace Engine::LuaAPI
