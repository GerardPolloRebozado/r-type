#include "Scripting/LuaAPI/TimeAPI.hpp"
#include "Scripting/LuaAPI/Internal/Helpers.hpp"
#include <raylib.h>
#include <sol/sol.hpp>

namespace Engine::LuaAPI {

void registerTime(sol::state_view lua) {
    sol::table t = Internal::ensureNamespace(lua, "Time");

    // Scripts should prefer the dt passed to onUpdate; this is only a fallback
    t["deltaTime"] = []() -> float { return 1.0f / 60.0f; };
    t["time"] = []() -> float { return static_cast<float>(GetTime()); };
}

} // namespace Engine::LuaAPI
