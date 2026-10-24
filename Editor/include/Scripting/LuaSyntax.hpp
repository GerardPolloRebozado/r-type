#ifndef EDITOR_SCRIPTING_LUA_SYNTAX_HPP_
#define EDITOR_SCRIPTING_LUA_SYNTAX_HPP_

#include <string>
#include <vector>

namespace Editor {

/**
 * @brief One compile-time diagnostic for a Lua source buffer.
 *
 * Line numbers are 1-based, matching the Lua compiler and TextEditor error
 * markers. Line 1 is used when the compiler message cannot be parsed.
 */
struct LuaDiagnostic {
    int line = 1;
    std::string message;
};

/**
 * @brief Compiles a Lua chunk without running it.
 *
 * The chunk is loaded with luaL_loadbuffer and the resulting function is
 * discarded. Nothing from the script is executed, so this is safe to call on
 * unsaved editor buffers.
 *
 * @param source Lua source text.
 * @param chunkName Name shown by Lua in error messages, usually the project
 *        relative script path.
 * @return Zero or more syntax diagnostics. Lua normally stops at the first
 *         syntax error, so the vector usually has one entry.
 */
std::vector<LuaDiagnostic> CheckLuaSyntax(const std::string& source, const std::string& chunkName);

} // namespace Editor

#endif // EDITOR_SCRIPTING_LUA_SYNTAX_HPP_
