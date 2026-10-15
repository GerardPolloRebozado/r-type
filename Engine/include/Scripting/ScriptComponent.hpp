#ifndef ENGINE_SCRIPTING_SCRIPTCOMPONENT_HPP_
#define ENGINE_SCRIPTING_SCRIPTCOMPONENT_HPP_

#include <sol/sol.hpp>
#include <string>

namespace Engine {
/**
 * @struct ScriptComponent
 * @brief Component that attaches a Lua script to an entity.
 *
 * The script should return a table with optional lifecycle functions:
 *   - onCreate(entity)
 *   - onUpdate(entity, dt)
 *   - onDestroy(entity)
 */
struct ScriptComponent {
    /** @brief Path to the Lua script file, relative to the project folder. */
    std::string scriptPath;

    /** @brief If true, the script will be reloaded automatically when the file changes. */
    bool autoReload = true;

    /** @brief Internal: whether the script has been loaded successfully. */
    bool loaded = false;

    /** @brief Internal: sol2 table handle returned by the script. */
    sol::table scriptTable = sol::lua_nil;

    ScriptComponent() = default;
    explicit ScriptComponent(const std::string& path, bool autoReload_ = true)
        : scriptPath(path), autoReload(autoReload_) {}
};
} // namespace Engine

#endif // ENGINE_SCRIPTING_SCRIPTCOMPONENT_HPP_
