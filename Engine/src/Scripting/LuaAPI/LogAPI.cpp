#include "Scripting/LuaAPI/LogAPI.hpp"
#include "Logging/Logger.hpp"
#include <sol/sol.hpp>
#include <string>

namespace Engine::LuaAPI::Internal {
sol::table ensureNamespace(sol::state_view lua, const char* name);
}

namespace Engine::LuaAPI {

void registerLog(sol::state_view lua) {
    sol::table logTable = Internal::ensureNamespace(lua, "log");

    logTable["info"] = [](const std::string& msg) { log_info(msg); };
    logTable["error"] = [](const std::string& msg) { log_user_error(msg); };
    logTable["debug"] = [](const std::string& msg) { log_debug(msg); };

    lua["print"] = [](sol::variadic_args va) {
        std::string out;
        for (auto v : va) {
            out += v.get<std::string>() + "\t";
        }
        log_info(out);
    };
}

} // namespace Engine::LuaAPI
