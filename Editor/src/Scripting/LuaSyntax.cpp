#include "Scripting/LuaSyntax.hpp"

#include <cctype>
#include <lua.hpp>
#include <utility>

namespace Editor {
namespace {

struct ParsedError {
    int line = 1;
    std::string message;
};

ParsedError ParseLuaError(const std::string& error) {
    ParsedError parsed;
    parsed.message = error;

    for (std::size_t i = 0; i + 2 < error.size(); ++i) {
        if (error[i] != ':' || !std::isdigit(static_cast<unsigned char>(error[i + 1]))) {
            continue;
        }

        std::size_t end = i + 1;
        while (end < error.size() && std::isdigit(static_cast<unsigned char>(error[end]))) {
            ++end;
        }
        if (end >= error.size() || error[end] != ':') {
            continue;
        }

        try {
            parsed.line = std::stoi(error.substr(i + 1, end - i - 1));
        } catch (...) {
            parsed.line = 1;
        }
        parsed.message = error.substr(end + 1);
        const std::size_t first = parsed.message.find_first_not_of(" \t\r\n");
        parsed.message = first == std::string::npos ? std::string{} : parsed.message.substr(first);
        return parsed;
    }

    return parsed;
}

} // namespace

std::vector<LuaDiagnostic> CheckLuaSyntax(const std::string& source, const std::string& chunkName) {
    std::vector<LuaDiagnostic> diagnostics;

    lua_State* state = luaL_newstate();
    if (state == nullptr) {
        diagnostics.push_back({1, "Could not create a Lua state for syntax checking."});
        return diagnostics;
    }

    const std::string chunk = "@" + chunkName;
    const int status = luaL_loadbuffer(state, source.data(), source.size(), chunk.c_str());
    if (status != LUA_OK) {
        const char* error = lua_tostring(state, -1);
        if (error == nullptr) {
            diagnostics.push_back({1, "Lua reported a syntax error without a message."});
        } else {
            ParsedError parsed = ParseLuaError(error);
            diagnostics.push_back({parsed.line, std::move(parsed.message)});
        }
    }

    lua_close(state);
    return diagnostics;
}

} // namespace Editor
